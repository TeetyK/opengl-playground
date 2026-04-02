#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Windows.h"
#include "Shader.h"
#include <vector>
#include <cmath>

// Constants for mathematics
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const char* vertexShader3D = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec3 LocalPos; // Added for gradient

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    LocalPos = aPos; // Pass local position to fragment shader
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)glsl";

const char* fragmentShader3D = R"glsl(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 LocalPos;

uniform vec3 colorTop;
uniform vec3 colorBottom;
uniform bool useLighting;

void main()
{
    // Generate gradient based on local Y position
    // Assuming local y ranges from -50.0 to 50.0 based on our sphere radius
    float t = (LocalPos.y + 50.0) / 100.0;
    t = clamp(t, 0.0, 1.0);
    vec3 resultColor = mix(colorBottom, colorTop, t);

    if (useLighting) {
        // Basic directional lighting
        vec3 norm = length(Normal) > 0.0 ? normalize(Normal) : vec3(0.0, 1.0, 0.0);
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0); // white light

        vec3 ambient = 0.6 * vec3(1.0);

        resultColor = (ambient + diffuse) * resultColor;
    }

    FragColor = vec4(resultColor, 1.0);
}
)glsl";

int main() {
    Windows gameWindow(1280, 720, "3D Model Viewer");
    if (!gameWindow.init()) {
        return -1;
    }

    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);

    Shader shader3D(vertexShader3D, fragmentShader3D);

    // Generate Sphere Mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float radius = 50.0f;
    int sectorCount = 36;
    int stackCount = 18;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(z); // Swapped z and y to align with OpenGL y-up
            vertices.push_back(y);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = z * lengthInv; // Swapped z and y
            nz = y * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texCoords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    float lastFrameTime = 0.0f;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(gameWindow.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Camera settings
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 200.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;
    double lastX = gameWindow.getWidth() / 2.0;
    double lastY = gameWindow.getHeight() / 2.0;
    bool firstMouse = true;
    bool isMiddleMousePressed = false;

    // Character position and rotation
    glm::vec3 modelPos(0.0f, 0.0f, 0.0f);
    glm::mat4 modelRotation = glm::mat4(1.0f);
    float cameraSpeed = 150.0f;

    glm::vec3 colorTop(0.8f, 0.2f, 0.2f);
    glm::vec3 colorBottom(0.2f, 0.2f, 0.8f);
    bool useLighting = true;

    bool isLeftMousePressed = false;
    double lastLeftX = 0.0;
    double lastLeftY = 0.0;

    while (!gameWindow.shouldClose()) {
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        gameWindow.pollEvents();

        // Process Keyboard Input (Camera Movement)
        float velocity = cameraSpeed * deltaTime;
        if (gameWindow.isKeyPressed(GLFW_KEY_W)) {
            cameraPos += cameraFront * velocity;
        }
        if (gameWindow.isKeyPressed(GLFW_KEY_S)) {
            cameraPos -= cameraFront * velocity;
        }
        if (gameWindow.isKeyPressed(GLFW_KEY_A)) {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
        }
        if (gameWindow.isKeyPressed(GLFW_KEY_D)) {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
        }

        // Process Mouse Input (Camera Rotation)
        double xpos, ypos;
        gameWindow.getMousePosition(xpos, ypos);

        if (gameWindow.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
            if (!isMiddleMousePressed) {
                isMiddleMousePressed = true;
                firstMouse = true; // Reset to avoid jump
            }

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            // Make sure that when pitch is out of bounds, screen doesn't get flipped
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);
        } else {
            isMiddleMousePressed = false;
        }

        // Process Left Mouse Input (Sphere Rotation)
        if (gameWindow.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (!isLeftMousePressed) {
                isLeftMousePressed = true;
                lastLeftX = xpos;
                lastLeftY = ypos;
            } else {
                float xoffset = xpos - lastLeftX;
                float yoffset = ypos - lastLeftY; // Notice y direction
                lastLeftX = xpos;
                lastLeftY = ypos;

                float sensitivity = 0.5f;
                // Rotate around world Y and X axes based on mouse movement
                glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset * sensitivity), glm::vec3(1.0f, 0.0f, 0.0f));
                glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset * sensitivity), glm::vec3(0.0f, 1.0f, 0.0f));

                // Pre-multiply to apply rotation globally
                modelRotation = rotY * rotX * modelRotation;
            }
        } else {
            isLeftMousePressed = false;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Top Color", &colorTop[0]);
        ImGui::ColorEdit3("Bottom Color", &colorBottom[0]);
        ImGui::Checkbox("Use Lighting", &useLighting);
        ImGui::End();

        shader3D.use();

        // Update shader uniforms
        shader3D.setVec3("colorTop", colorTop);
        shader3D.setVec3("colorBottom", colorBottom);
        shader3D.setBool("useLighting", useLighting);

        // View/Projection setup
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gameWindow.getWidth() / (float)gameWindow.getHeight(), 0.1f, 200000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        shader3D.setMat4("projection", &projection[0][0]);
        shader3D.setMat4("view", &view[0][0]);

        // Model matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, modelPos);
        modelMatrix = modelMatrix * modelRotation;

        shader3D.setMat4("model", &modelMatrix[0][0]);

        // Draw sphere
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        gameWindow.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
