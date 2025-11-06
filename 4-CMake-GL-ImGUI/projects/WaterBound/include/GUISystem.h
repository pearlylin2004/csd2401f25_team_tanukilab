#pragma once
#include "GameObjectManager.h"
#include "input.h"
#include <string>
#include <unordered_map>

enum class GameState {
    MENU,
    PLAYING
};

struct ButtonData {
    glm::vec3 position;
    glm::vec3 scale;
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

    // New method to load button data from JSON
    bool loadButtonDataFromJSON(const std::string& filename);

    // Store button data
    std::unordered_map<std::string, ButtonData> m_buttonData;

    bool isPointInButton(const glm::vec2& point, GameObject* button);

    GameState m_currentState = GameState::MENU;
    GameObject* m_playButton = nullptr;
    GameObject* m_stopButton = nullptr;
    bool m_buttonsCreated = false;
    bool m_buttonDataLoaded = false;
};