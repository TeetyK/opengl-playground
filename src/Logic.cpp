#include "Logic.h"
#include <imgui.h>
#include <string>
#include <cstring>

Logic::Logic()
    : currentState(GameState::Menu), speed(100.0f), showCommandPrompt(false),
      justOpenedPrompt(false), wasSlashPressed(false), wasEscPressed(false) {
    commandBuf[0] = '\0';
}

Logic::~Logic() {}

void Logic::update(float deltaTime, Windows& window, Character& character) {
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
        updatePlayingState(deltaTime, window, character);
    }
}

void Logic::updatePlayingState(float deltaTime, Windows& window, Character& character) {
    if (!showCommandPrompt) {
        float moveSpeed = speed * deltaTime;

        // Mouse interaction example
        double mx, my;
        window.getMousePosition(mx, my);
        bool clicked = window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        // Could use mx, my, clicked for something, currently just acknowledging it's available.

        if (window.isKeyPressed(GLFW_KEY_W)) character.y += moveSpeed;
        if (window.isKeyPressed(GLFW_KEY_S)) character.y -= moveSpeed;
        if (window.isKeyPressed(GLFW_KEY_A)) character.x -= moveSpeed;
        if (window.isKeyPressed(GLFW_KEY_D)) character.x += moveSpeed;

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
    if (ImGui::Button("Back", ImVec2(200, 50))) {
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
