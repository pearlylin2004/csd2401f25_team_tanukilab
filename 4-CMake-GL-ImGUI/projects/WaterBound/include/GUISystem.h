/* Start Header ************************************************************************/
/*!
\file       GUISystem.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 90% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\date       November, 7th, 2025
\brief      Consists of declarations for managing creation and removal
//          of menu buttons, detects button clicks, and updates game state
//          accordingly.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "GameObjectManager.h"
#include "input.h"
#include <string>
#include <unordered_map>

class LuaSystem;

enum class GameState {
    MENU,
    PLAYING
};

// ButtonData is now defined in GameObjectManager.h - no need to redefine it here

class GUISystem {
public:
    GUISystem();
    ~GUISystem();

    void init(GameObjectManager& manager, LuaSystem& luaSystem);
    void update(GameObjectManager& manager);

    bool isPlaying() const { return m_currentState == GameState::PLAYING; }
    GameState getCurrentState() const { return m_currentState; }

private:
    void createMenuButtons(GameObjectManager& manager);
    void removeMenuButtons(GameObjectManager& manager);
    void startGame(GameObjectManager& manager);
    void stopGame(GameObjectManager& manager);

    bool isPointInButton(const glm::vec2& point, GameObject* button);

    // Store button data retrieved from GameObjectManager
    std::unordered_map<std::string, ButtonData> m_buttonData;

    GameState m_currentState = GameState::MENU;
    GameObject* m_playButton = nullptr;
    GameObject* m_stopButton = nullptr;
    bool m_buttonsCreated = false;
    GameObjectManager* g_manager = nullptr;
    LuaSystem* g_luaSystem = nullptr;
    bool m_buttonDataLoaded = false;
};