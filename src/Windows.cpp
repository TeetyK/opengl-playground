#include "Windows.h"
#include <iostream>

Windows::Windows(int w, int h, const char* t) : width(w), height(h), title(t), window(nullptr) {}

Windows::~Windows() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Windows::init() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

bool Windows::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Windows::swapBuffers() const {
    glfwSwapBuffers(window);
}

void Windows::pollEvents() const {
    glfwPollEvents();
}

void Windows::close() const {
    glfwSetWindowShouldClose(window, true);
}

GLFWwindow* Windows::getWindow() const {
    return window;
}

bool Windows::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Windows::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Windows::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(window, &x, &y);
}

void Windows::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
