#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>

class Mesh {
public:
    Mesh(float* vertices, int vertexSize, unsigned int* indices, int indexSize);
    ~Mesh();

    void draw() const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    int indexCount;
};

#endif
