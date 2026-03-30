#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Windows.h"
#include "Shader.h"
#include "Mesh.h"
#include "Character.h"
#include "Logic.h"

// Vertex shader source
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform float offsetX;
uniform float offsetY;
uniform float offsetZ;
uniform float scaleX;
uniform float scaleY;

void main()
{
    vec4 pos = vec4((aPos.x * scaleX) + offsetX, (aPos.y * scaleY) + offsetY + offsetZ, aPos.z, 1.0);
    gl_Position = projection * pos;
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

int main() {
    Windows gameWindow(1280, 600, "2.5D Playground Game");
    if (!gameWindow.init()) {
        return -1;
    }

    // Set up model vertices (a simple quad, size 1x1)
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

    Mesh myMesh(vertices, sizeof(vertices), indices, sizeof(indices));
    Shader myShader(vertexShaderSource, fragmentShaderSource);

    // Setup character at center (640, 300) with scale 50x50 pixels
    Character myCharacter(640.0f, 300.0f, 0.0f, 50.0f, 50.0f, &myMesh);

    Logic gameLogic;

    // Setup projection matrix
    glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    myShader.use();
    myShader.setMat4("projection", &projection[0][0]);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(gameWindow.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float lastFrameTime = 0.0f;

    while (!gameWindow.shouldClose()) {
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        gameWindow.pollEvents();

        gameLogic.update(deltaTime, gameWindow, myCharacter);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        gameLogic.renderUI(gameWindow);

        if (gameLogic.getGameState() == GameState::Playing) {
            myCharacter.draw(myShader);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        gameWindow.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}