#include "Logic.h"
#include <imgui.h>
#include <string>
#include <cstring>

Logic::Logic()
    : currentState(GameState::Menu), speed(100.0f), showCommandPrompt(false),
      justOpenedPrompt(false), wasSlashPressed(false), wasEscPressed(false), playerName("Player") {
    commandBuf[0] = '\0';
    strncpy(nameBuf, playerName.c_str(), sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0';
}

Logic::~Logic() {}

void Logic::update(float deltaTime, Windows& window, Character& character, const std::vector<std::vector<int>>& gameMap, float tileSize) {
    bool isEscPressed = window.isKeyPressed(GLFW_KEY_ESCAPE);
    if (isEscPressed && !wasEscPressed) {
        if (showCommandPrompt) {
            showCommandPrompt = false;
        } else if (currentState == GameState::Playing) {
            currentState = GameState::Menu;
        } else {
            window.close();
        }
    }
    wasEscPressed = isEscPressed;

    if (currentState == GameState::Playing) {
        updatePlayingState(deltaTime, window, character, gameMap, tileSize);
    }
}

void Logic::updatePlayingState(float deltaTime, Windows& window, Character& character, const std::vector<std::vector<int>>& gameMap, float tileSize) {
    if (!showCommandPrompt) {
        float moveSpeed = speed * deltaTime;

        // Mouse interaction example
        double mx, my;
        window.getMousePosition(mx, my);
        bool clicked = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        // Could use mx, my, clicked for something, currently just acknowledging it's available.

        bool moving = false;
        float nextX = character.x;
        float nextY = character.y;

        if (window.isKeyPressed(GLFW_KEY_W)) { nextY += moveSpeed; character.direction = 1; moving = true; }
        if (window.isKeyPressed(GLFW_KEY_S)) { nextY -= moveSpeed; character.direction = 0; moving = true; }
        if (window.isKeyPressed(GLFW_KEY_A)) { nextX -= moveSpeed; character.direction = 2; character.flipX = true; moving = true; }
        if (window.isKeyPressed(GLFW_KEY_D)) { nextX += moveSpeed; character.direction = 2; character.flipX = false; moving = true; }

        if (moving) {
            // Collision detection
            // Calculate grid indices based on next position.
            // We use center of character or bottom, let's just use nextX, nextY directly.
            int gridX = static_cast<int>(nextX / tileSize);
            int gridY = static_cast<int>(nextY / tileSize);

            // Clamp indices
            if (gridY >= 0 && gridY < gameMap.size() && gridX >= 0 && gridX < gameMap[0].size()) {
                if (gameMap[gridY][gridX] == 0) { // 0 is walkable
                    character.x = nextX;
                    character.y = nextY;
                }
            }
        }

        character.isMoving = moving;

        if (window.isKeyPressed(GLFW_KEY_SPACE) && !character.isJumping) {
            character.isJumping = true;
            character.velocityZ = 500.0f; // Scale up jump for ortho projection
        }

        // Clamp character position to 1280x600 (walkable area)
        // Assume character's x,y represents center or bottom. Let's clamp to screen bounds 0-1280, 0-600.
        // Or if centered: -640 to 640 and -300 to 300 depending on camera. We'll assume bottom-left 0,0 mapping.
        if (character.x < 0) character.x = 0;
        if (character.x > window.getWidth()) character.x = static_cast<float>(window.getWidth());
        if (character.y < 0) character.y = 0;
        if (character.y > window.getHeight()) character.y = static_cast<float>(window.getHeight());
    }

    bool isSlashPressed = window.isKeyPressed(GLFW_KEY_SLASH);
    if (isSlashPressed && !wasSlashPressed && !showCommandPrompt) {
        showCommandPrompt = true;
        justOpenedPrompt = true;
        memset(commandBuf, 0, sizeof(commandBuf));
    }
    wasSlashPressed = isSlashPressed;

    character.update(deltaTime);
}

void Logic::renderUI(Windows& window) {
    if (currentState == GameState::Menu) {
        renderMainMenu(window);
    } else if (currentState == GameState::Options) {
        renderOptionsMenu(window);
    } else if (currentState == GameState::Playing) {
        renderPlayingUI(window);
    }
}

void Logic::renderMainMenu(Windows& window) {
    ImGui::SetNextWindowPos(ImVec2(window.getWidth() / 2.0f, window.getHeight() / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

    if (ImGui::Button("Start", ImVec2(200, 50))) {
        currentState = GameState::Playing;
    }
    if (ImGui::Button("Options", ImVec2(200, 50))) {
        currentState = GameState::Options;
    }
    if (ImGui::Button("Exit", ImVec2(200, 50))) {
        window.close();
    }

    ImGui::End();
}

void Logic::renderOptionsMenu(Windows& window) {
    ImGui::SetNextWindowPos(ImVec2(window.getWidth() / 2.0f, window.getHeight() / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

    ImGui::Text("Options Menu");

    ImGui::InputText("Player Name", nameBuf, sizeof(nameBuf));
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        playerName = std::string(nameBuf);
    }

    if (ImGui::Button("Back", ImVec2(200, 50))) {
        playerName = std::string(nameBuf); // Save on exit
        currentState = GameState::Menu;
    }

    ImGui::End();
}

void Logic::renderPlayingUI(Windows& window) {
    if (showCommandPrompt) {
        ImGui::SetNextWindowPos(ImVec2(0, window.getHeight() - 40.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window.getWidth()), 40.0f), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
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
}
