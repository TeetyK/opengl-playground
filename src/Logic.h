#ifndef LOGIC_H
#define LOGIC_H

#include "Windows.h"
#include "Character.h"

enum class GameState {
    Menu,
    Playing,
    Options
};

class Logic {
public:
    Logic();
    ~Logic();

    void update(float deltaTime, Windows& window, Character& character);
    void renderUI(Windows& window);

    GameState getGameState() const { return currentState; }

private:
    GameState currentState;
    float speed;
    bool showCommandPrompt;
    char commandBuf[256];
    bool justOpenedPrompt;
    bool wasSlashPressed;
    bool wasEscPressed;

    void updatePlayingState(float deltaTime, Windows& window, Character& character);
    void renderMainMenu(Windows& window);
    void renderOptionsMenu(Windows& window);
    void renderPlayingUI(Windows& window);
};

#endif
