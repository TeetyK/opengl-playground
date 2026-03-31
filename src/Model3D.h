#ifndef MODEL3D_H
#define MODEL3D_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Image.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    std::shared_ptr<Image> image;
    std::string type;
    std::string path;
};

class Mesh3D {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    glm::vec3 diffuseColor;

    Mesh3D(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 diffuseColor = glm::vec3(1.0f));
    ~Mesh3D();

    // Copy construction and assignment are deleted to easily manage GL resources or implemented via rule of 5
    // For simplicity here, we'll allow moving or just assume they are stored in a vector without copy

    void draw(Shader &shader, glm::vec3 overrideColor = glm::vec3(1.0f)) const;
    void setupMesh();

private:
    unsigned int VAO, VBO, EBO;
};

class Model3D {
public:
    Model3D(const std::string& path, const std::string& baseDir);
    void draw(Shader &shader, glm::vec3 overrideColor = glm::vec3(1.0f)) const;

private:
    std::vector<Mesh3D> meshes;
    std::string directory;
    std::map<std::string, std::shared_ptr<Image>> loadedTextures;

    void loadModel(const std::string& path, const std::string& baseDir);
    std::shared_ptr<Image> loadTexture(const std::string& path);
};

#endif // MODEL3D_H
