#include "Model3D.h"
#include <glad/glad.h>
#include <iostream>

#include <tiny_obj_loader.h>

Mesh3D::Mesh3D(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 diffuseColor)
    : vertices(vertices), indices(indices), textures(textures), diffuseColor(diffuseColor), VAO(0), VBO(0), EBO(0) {
    setupMesh();
}

Mesh3D::~Mesh3D() {
    // If we wanted to manage destructors perfectly we would need move semantics.
    // For now we assume they live as long as the Model3D.
}

void Mesh3D::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh3D::draw(Shader &shader, glm::vec3 overrideColor) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    shader.setBool("useTexture", textures.size() > 0);
    shader.setVec3("colorModifier", diffuseColor * overrideColor);

    for(unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.setInt(("material." + name + number).c_str(), i);
        textures[i].image->bind();
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Model3D::Model3D(const std::string& path, const std::string& baseDir) {
    directory = baseDir;
    loadModel(path, baseDir);
}

void Model3D::draw(Shader &shader, glm::vec3 overrideColor) const {
    for(unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].draw(shader, overrideColor);
    }
}

void Model3D::loadModel(const std::string& path, const std::string& baseDir) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = baseDir;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    for (size_t s = 0; s < shapes.size(); s++) {
        // We will create a separate Mesh3D for each material in this shape.
        // A shape might have faces with different materials.
        std::map<int, std::vector<Vertex>> mat_vertices;
        std::map<int, std::vector<unsigned int>> mat_indices;

        // tinyobjloader uses separate indices for v, vn, vt.
        // We need to flatten them for OpenGL. To do this efficiently, we typically use a map,
        // but for simplicity, we can just push every vertex and give it a new index per material.
        size_t index_offset = 0;

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            int current_material_id = shapes[s].mesh.material_ids[f];

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                Vertex vertex;

                vertex.Position = {
                    attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 2]
                };

                if (idx.normal_index >= 0) {
                    vertex.Normal = {
                        attrib.normals[3 * size_t(idx.normal_index) + 0],
                        attrib.normals[3 * size_t(idx.normal_index) + 1],
                        attrib.normals[3 * size_t(idx.normal_index) + 2]
                    };
                } else {
                    vertex.Normal = {0.0f, 0.0f, 0.0f};
                }

                if (idx.texcoord_index >= 0) {
                    vertex.TexCoords = {
                        attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
                        1.0f - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1] // flip y
                    };
                } else {
                    vertex.TexCoords = {0.0f, 0.0f};
                }

                mat_vertices[current_material_id].push_back(vertex);
                mat_indices[current_material_id].push_back(mat_vertices[current_material_id].size() - 1);
            }
            index_offset += fv;
        }

        // Now create a mesh for each material found in this shape
        for (const auto& pair : mat_vertices) {
            int mat_id = pair.first;
            std::vector<Texture> textures;
            glm::vec3 diffuseColor(1.0f);

            if (mat_id >= 0 && mat_id < materials.size()) {
                const auto& mat = materials[mat_id];
                diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

                if (!mat.diffuse_texname.empty()) {
                    std::string texPath = directory + "/" + mat.diffuse_texname;
                    auto img = loadTexture(texPath);
                    if (img) {
                        Texture texture;
                        texture.image = img;
                        texture.type = "texture_diffuse";
                        texture.path = mat.diffuse_texname;
                        textures.push_back(texture);
                    }
                }
            }

            meshes.push_back(Mesh3D(mat_vertices[mat_id], mat_indices[mat_id], textures, diffuseColor));
        }
    }
}

std::shared_ptr<Image> Model3D::loadTexture(const std::string& path) {
    if (loadedTextures.find(path) != loadedTextures.end()) {
        return loadedTextures[path];
    }

    auto img = std::make_shared<Image>(path);
    loadedTextures[path] = img;
    return img;
}
