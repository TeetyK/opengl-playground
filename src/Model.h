#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

class Model {
public:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    int indexCount;

    Model(float* vertices, int vertexSize, unsigned int* indices, int indexSize);
    ~Model();

    void draw() const;
};

#endif
