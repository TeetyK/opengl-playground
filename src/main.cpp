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
#include "Image.h"

// Vertex shader source
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform float offsetX;
uniform float offsetY;
uniform float offsetZ;
uniform float scaleX;
uniform float scaleY;
uniform vec2 texOffset;
uniform vec2 texScale;

void main()
{
    vec4 pos = vec4((aPos.x * scaleX) + offsetX, (aPos.y * scaleY) + offsetY + offsetZ, aPos.z, 1.0);
    gl_Position = projection * pos;
    TexCoord = vec2((aTexCoord.x * texScale.x) + texOffset.x, (aTexCoord.y * texScale.y) + texOffset.y);
}
)glsl";

// Fragment shader source
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;
uniform vec4 colorModifier;

void main()
{
    if (useTexture) {
        vec4 texColor = texture(ourTexture, TexCoord);
        if (texColor.a < 0.1)
            discard;
        FragColor = texColor * colorModifier;
    } else {
        FragColor = colorModifier;
    }
}
)glsl";

int main() {
    Windows gameWindow(1280, 600, "2.5D Playground Game");
    if (!gameWindow.init()) {
        return -1;
    }

    // Set up model vertices (a simple quad, size 1x1, with UVs)
    float vertices[] = {
        // positions         // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    Mesh myMesh(vertices, sizeof(vertices), indices, sizeof(indices));
    Shader myShader(vertexShaderSource, fragmentShaderSource);

    // Load character textures
    Image charIdle("assets/Character/Main/Idle/Character_down_idle-Sheet6.png");
    Image charRun("assets/Character/Main/Run/Character_down_run-Sheet6.png");

    // Load background and environment textures
    Image backgroundTex("assets/Tiles/Background_Green_TileSet.png");
    Image bushTex("assets/Objects/Nature/Green/Bush_1_Green.png");

    // Setup character at center (640, 300) with scale 50x50 pixels
    Character myCharacter(640.0f, 300.0f, 0.0f, 50.0f, 50.0f, &myMesh);
    myCharacter.idleTexture = &charIdle;
    myCharacter.runTexture = &charRun;

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
            myShader.use();

            // Draw Background
            glActiveTexture(GL_TEXTURE0);
            backgroundTex.bind();
            myShader.setInt("ourTexture", 0);
            myShader.setBool("useTexture", true);
            myShader.setVec4("colorModifier", 1.0f, 1.0f, 1.0f, 1.0f);

            // Tile the background across the screen (1280x600)
            // The texture is 384x272. We can draw one large quad.
            myShader.setFloat("offsetX", 640.0f);
            myShader.setFloat("offsetY", 300.0f);
            myShader.setFloat("offsetZ", -0.5f); // push behind everything
            myShader.setFloat("scaleX", 1280.0f);
            myShader.setFloat("scaleY", 600.0f);
            // Repeat UVs
            myShader.setVec2("texOffset", 0.0f, 0.0f);
            myShader.setVec2("texScale", 1280.0f / 384.0f, 600.0f / 272.0f);
            myMesh.draw();
            backgroundTex.unbind();

            struct RenderEntity {
                float x, y, z;
                int type; // 0 = character, 1 = bush
            };

            std::vector<RenderEntity> entities = {
                {myCharacter.x, myCharacter.y, myCharacter.z, 0},
                {200.0f, 400.0f, 0.0f, 1},
                {800.0f, 200.0f, 0.0f, 1}
            };

            // Sort by Y descending for 2.5D top-down perspective
            std::sort(entities.begin(), entities.end(), [](const RenderEntity& a, const RenderEntity& b) {
                return a.y > b.y;
            });

            for (const auto& ent : entities) {
                if (ent.type == 0) {
                    // Draw character
                    myCharacter.draw(myShader);
                    myShader.use(); // Re-use shader setup for next items
                } else if (ent.type == 1) {
                    // Draw Bush
                    bushTex.bind();
                    myShader.setFloat("scaleX", 50.0f);
                    myShader.setFloat("scaleY", 50.0f);
                    myShader.setVec2("texOffset", 0.0f, 0.0f);
                    myShader.setVec2("texScale", 1.0f, 1.0f);
                    myShader.setBool("useTexture", true);
                    myShader.setInt("ourTexture", 0);

                    myShader.setFloat("offsetX", ent.x);
                    myShader.setFloat("offsetY", ent.y);
                    myShader.setFloat("offsetZ", ent.z);
                    myMesh.draw();
                    bushTex.unbind();
                }
            }

            // Draw Player Name above character
            std::string pName = gameLogic.getPlayerName();
            if (!pName.empty()) {
                // Determine text size to center it
                ImVec2 textSize = ImGui::CalcTextSize(pName.c_str());
                // The character's y is standard OpenGL, ImGui Y is inverted
                // So Character (x, y) needs to be converted. Since projection is ortho(0, width, 0, height)
                // character.x is already screen X. character.y is from bottom. ImGui is from top.
                float screenY = gameWindow.getHeight() - (myCharacter.y + myCharacter.height/2.0f + 20.0f);
                float screenX = myCharacter.x - (textSize.x / 2.0f);

                ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenX, screenY), IM_COL32(255, 255, 255, 255), pName.c_str());
            }
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