#ifndef CHARACTER_H
#define CHARACTER_H

#include "Mesh.h"
#include "Shader.h"
#include "Image.h"

class Character {
public:
    float x, y, z;
    float width, height;
    float velocityZ;
    bool isJumping;
    Mesh* model;

    Image* idleTexture;
    Image* runTexture;

    int currentFrame;
    float animationTime;
    float frameDuration;
    int numFrames;
    int direction; // 0=down, 1=up, 2=side (left/right)
    bool isMoving;
    bool flipX;

    Character(float startX, float startY, float startZ, float w, float h, Mesh* charModel);
    void update(float deltaTime);
    void draw(Shader& shader);
};

#endif
