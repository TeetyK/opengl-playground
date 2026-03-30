#include "Character.h"

Character::Character(float startX, float startY, float startZ, float w, float h, Mesh* charModel)
    : x(startX), y(startY), z(startZ), width(w), height(h), model(charModel), velocityZ(0.0f), isJumping(false),
      idleTexture(nullptr), runTexture(nullptr), currentFrame(0), animationTime(0.0f), frameDuration(0.1f), numFrames(6), direction(0), isMoving(false), flipX(false) {
}

void Character::update(float deltaTime) {
    if (isJumping) {
        float gravity = -980.0f; // Scaled up for screen pixel coordinates
        velocityZ += gravity * deltaTime;
        z += velocityZ * deltaTime;

        if (z <= 0.0f) {
            z = 0.0f;
            isJumping = false;
            velocityZ = 0.0f;
        }
    }

    animationTime += deltaTime;
    if (animationTime >= frameDuration) {
        animationTime = 0.0f;
        currentFrame = (currentFrame + 1) % numFrames;
    }
}

void Character::draw(Shader& shader) {
    if (model) {
        shader.use();
        shader.setFloat("offsetX", x);
        shader.setFloat("offsetY", y);
        shader.setFloat("offsetZ", z);

        // Handling sprite sheet (6 frames horizontal)
        float uOffset = static_cast<float>(currentFrame) / static_cast<float>(numFrames);
        float vOffset = 0.0f;
        float uScale = 1.0f / static_cast<float>(numFrames);
        float vScale = 1.0f;

        // Apply flipX
        float actualScaleX = width;
        if (flipX) {
            actualScaleX = -width;
        }

        shader.setFloat("scaleX", actualScaleX);
        shader.setFloat("scaleY", height);

        shader.setVec2("texOffset", uOffset, vOffset);
        shader.setVec2("texScale", uScale, vScale);
        shader.setBool("useTexture", true);
        shader.setVec4("colorModifier", 1.0f, 1.0f, 1.0f, 1.0f);

        Image* currentTex = isMoving ? runTexture : idleTexture;
        if (currentTex) {
            glActiveTexture(GL_TEXTURE0);
            currentTex->bind();
            shader.setInt("ourTexture", 0);
        } else {
            shader.setBool("useTexture", false);
        }

        model->draw();

        if (currentTex) {
            currentTex->unbind();
        }
    }
}
