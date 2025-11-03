#pragma once
#include "GameObjectManager.h"
#include "input.h"

enum class GameState {
    MENU,
    PLAYING
};

class GUISystem {
public:
    GUISystem();
    ~GUISystem();

    void init(GameObjectManager& manager);
    void update(GameObjectManager& manager);

    bool isPlaying() const { return m_currentState == GameState::PLAYING; }
    GameState getCurrentState() const { return m_currentState; }

private:
    void createMenuButtons(GameObjectManager& manager);
    void removeMenuButtons(GameObjectManager& manager);
    void startGame(GameObjectManager& manager);
    void stopGame(GameObjectManager& manager);

    bool isPointInButton(const glm::vec2& point, GameObject* button);
    //glm::vec2 convertScreenToWorld(double screenX, double screenY);

    GameState m_currentState = GameState::MENU;
    GameObject* m_playButton = nullptr;
    GameObject* m_stopButton = nullptr;
    bool m_buttonsCreated = false;
};