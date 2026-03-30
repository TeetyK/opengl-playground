#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Shader.h"
#include "Model.h"
#include "Character.h"

enum class GameState {
    Menu,
    Playing,
    Options
};

// Vertex shader source
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform float offsetX;
uniform float offsetY;
uniform float offsetZ;
uniform float scaleX;
uniform float scaleY;

void main()
{
    gl_Position = vec4((aPos.x * scaleX) + offsetX, (aPos.y * scaleY) + offsetY + offsetZ, aPos.z, 1.0);
}
)glsl";

// Fragment shader source
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); // Orange color
}
)glsl";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up model vertices (a simple quad)
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    Model myModel(vertices, sizeof(vertices), indices, sizeof(indices));
    Shader myShader(vertexShaderSource, fragmentShaderSource);
    Character myCharacter(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, &myModel); // Centered, 50% scale

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GameState currentState = GameState::Menu;
    float speed = 1.0f;
    float lastFrameTime = 0.0f;
    bool showCommandPrompt = false;
    char commandBuf[256] = "";
    bool justOpenedPrompt = false;
    bool wasSlashPressed = false;
    bool wasEscPressed = false;

    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        bool isEscPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        if (isEscPressed && !wasEscPressed) {
            if (showCommandPrompt) {
                showCommandPrompt = false;
            } else {
                glfwSetWindowShouldClose(window, true);
            }
        }
        wasEscPressed = isEscPressed;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (currentState == GameState::Menu) {
            ImGui::SetNextWindowPos(ImVec2(800.0f / 2.0f, 600.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

            if (ImGui::Button("Start", ImVec2(200, 50))) {
                currentState = GameState::Playing;
            }
            if (ImGui::Button("Options", ImVec2(200, 50))) {
                currentState = GameState::Options;
            }
            if (ImGui::Button("Exit", ImVec2(200, 50))) {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::End();
        } else if (currentState == GameState::Options) {
            ImGui::SetNextWindowPos(ImVec2(800.0f / 2.0f, 600.0f / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

            ImGui::Text("Options Menu");
            // Add any options here
            if (ImGui::Button("Back", ImVec2(200, 50))) {
                currentState = GameState::Menu;
            }

            ImGui::End();
        } else if (currentState == GameState::Playing) {
            if (!showCommandPrompt) {
                float moveSpeed = speed * deltaTime;
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) myCharacter.y += moveSpeed;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) myCharacter.y -= moveSpeed;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) myCharacter.x -= moveSpeed;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) myCharacter.x += moveSpeed;

                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !myCharacter.isJumping) {
                    myCharacter.isJumping = true;
                    myCharacter.velocityZ = 5.0f; // Initial jump velocity
                }
            }

            // Handle slash press properly to avoid registering / inside input
            bool isSlashPressed = glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS;
            if (isSlashPressed && !wasSlashPressed && !showCommandPrompt) {
                showCommandPrompt = true;
                justOpenedPrompt = true;
                memset(commandBuf, 0, sizeof(commandBuf));
            }
            wasSlashPressed = isSlashPressed;

            if (showCommandPrompt) {
                ImGui::SetNextWindowPos(ImVec2(0, 600 - 40), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(800, 40), ImGuiCond_Always);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Gray background
                ImGui::Begin("Command Prompt", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                if (justOpenedPrompt) {
                    ImGui::SetKeyboardFocusHere();
                    justOpenedPrompt = false;
                }

                ImGui::PushItemWidth(-1);
                if (ImGui::InputText("##cmd", commandBuf, sizeof(commandBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    std::string cmd(commandBuf);
                    if (cmd.find("/speed ") == 0) {
                        try {
                            speed = std::stof(cmd.substr(7));
                        } catch (...) {
                            // Invalid number
                        }
                    }
                    showCommandPrompt = false;
                }
                ImGui::PopItemWidth();

                ImGui::End();
                ImGui::PopStyleColor();
            }

            myCharacter.update(deltaTime);

            // Render the character
            myCharacter.draw(myShader);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}