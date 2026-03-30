#ifndef WINDOWS_H
#define WINDOWS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Windows {
public:
    Windows(int width, int height, const char* title);
    ~Windows();

    bool init();
    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;
    void close() const;

    GLFWwindow* getWindow() const;

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getMousePosition(double& x, double& y) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    const char* title;
    GLFWwindow* window;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

#endif
