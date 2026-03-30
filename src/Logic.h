#ifndef LOGIC_H
#define LOGIC_H

#include "Windows.h"
#include "Character.h"
#include <vector>

enum class GameState {
    Menu,
    Playing,
    Options
};

class Logic {
public:
    Logic();
    ~Logic();

    void update(float deltaTime, Windows& window, Character& character, const std::vector<std::vector<int>>& gameMap, float tileSize);
    void renderUI(Windows& window);

    GameState getGameState() const { return currentState; }

    std::string getPlayerName() const { return playerName; }

private:
    GameState currentState;
    float speed;
    bool showCommandPrompt;
    char commandBuf[256];
    bool justOpenedPrompt;
    bool wasSlashPressed;
    bool wasEscPressed;

    char nameBuf[64];
    std::string playerName;

    void updatePlayingState(float deltaTime, Windows& window, Character& character, const std::vector<std::vector<int>>& gameMap, float tileSize);
    void renderMainMenu(Windows& window);
    void renderOptionsMenu(Windows& window);
    void renderPlayingUI(Windows& window);
};

#endif
