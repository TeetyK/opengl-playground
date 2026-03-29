#ifndef CHARACTER_H
#define CHARACTER_H

#include "Model.h"
#include "Shader.h"

class Character {
public:
    float x, y;
    float width, height;
    Model* model;

    Character(float startX, float startY, float w, float h, Model* charModel);
    void draw(Shader& shader);
};

#endif
