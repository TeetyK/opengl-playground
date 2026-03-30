#include "Character.h"

Character::Character(float startX, float startY, float startZ, float w, float h, Mesh* charModel)
    : x(startX), y(startY), z(startZ), width(w), height(h), model(charModel), velocityZ(0.0f), isJumping(false) {
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
}

void Character::draw(Shader& shader) {
    if (model) {
        // Here we could set uniforms for position/scale if we had a more complex shader.
        // For a simple 2D game playground, we might just draw the model directly if the model vertices
        // are already transformed, or use uniforms. Let's assume a basic shader that takes a translation uniform.

        // As a simple placeholder, we'll just set some uniforms. We will need our shader to support these.
        shader.use();
        shader.setFloat("offsetX", x);
        shader.setFloat("offsetY", y);
        shader.setFloat("offsetZ", z);
        shader.setFloat("scaleX", width);
        shader.setFloat("scaleY", height);

        model->draw();
    }
}
