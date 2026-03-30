#ifndef CHARACTER_H
#define CHARACTER_H

#include "Mesh.h"
#include "Shader.h"

class Character {
public:
    float x, y, z;
    float width, height;
    float velocityZ;
    bool isJumping;
    Mesh* model;

    Character(float startX, float startY, float startZ, float w, float h, Mesh* charModel);
    void update(float deltaTime);
    void draw(Shader& shader);
};

#endif
