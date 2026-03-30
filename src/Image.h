#ifndef IMAGE_H
#define IMAGE_H

#include <string>

class Image {
public:
    Image(const std::string& filepath);
    ~Image();

    void bind() const;
    void unbind() const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    unsigned int getTextureID() const { return textureID; }

private:
    unsigned int textureID;
    int width, height, channels;
    bool loaded;
};

#endif
