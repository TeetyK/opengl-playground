#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Windows.h"
#include "Shader.h"
#include "Model3D.h"

const char* vertexShader3D = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
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

struct Material {
    sampler2D texture_diffuse1;
};

uniform Material material;
uniform bool useTexture;
uniform vec3 colorModifier;

void main()
{
    vec3 resultColor;
    if (useTexture) {
        vec4 texColor = texture(material.texture_diffuse1, TexCoords);
        if(texColor.a < 0.1)
            discard;
        resultColor = texColor.rgb * colorModifier;
    } else {
        resultColor = colorModifier;
    }

    // Basic directional lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0); // white light

    vec3 ambient = 0.2 * vec3(1.0);

    vec3 finalColor = (ambient + diffuse) * resultColor;
    FragColor = vec4(finalColor, 1.0);
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

    // Load the 3D model
    std::cout << "Loading 3D model..." << std::endl;
    Model3D myModel("model/source/Koharu.obj", "model/source");
    std::cout << "Model loaded." << std::endl;

    float lastFrameTime = 0.0f;

    while (!gameWindow.shouldClose()) {
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        gameWindow.pollEvents();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader3D.use();

        // View/Projection setup
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gameWindow.getWidth() / (float)gameWindow.getHeight(), 0.1f, 100000.0f);

        // Setup camera
        float camX = sin(glfwGetTime() * 0.5f) * 15000.0f;
        float camZ = cos(glfwGetTime() * 0.5f) * 15000.0f;
        glm::mat4 view = glm::lookAt(glm::vec3(camX, 5000.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        shader3D.setMat4("projection", &projection[0][0]);
        shader3D.setMat4("view", &view[0][0]);

        // Model matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        // Translate model to center (arbitrary, based on model coords)
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        // Optional scale if the model is too big/small
        // modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f));
        shader3D.setMat4("model", &modelMatrix[0][0]);

        // Draw model
        myModel.draw(shader3D);

        gameWindow.swapBuffers();
    }

    return 0;
}
