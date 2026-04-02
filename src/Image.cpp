#include "Image.h"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Image::Image(const std::string& filepath) : textureID(0), width(0), height(0), channels(0), loaded(false) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    stbi_set_flip_vertically_on_load(true);
    // Force loading with 4 channels (RGBA) so textures correctly provide alpha channel for the shader discard check.
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (data) {
        GLenum format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        loaded = true;
    } else {
        std::cout << "Failed to load texture: " << filepath << std::endl;
    }
    stbi_image_free(data);
}

Image::~Image() {
    if (loaded) {
        glDeleteTextures(1, &textureID);
    }
}

void Image::bind() const {
    if (loaded) {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
}

void Image::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
