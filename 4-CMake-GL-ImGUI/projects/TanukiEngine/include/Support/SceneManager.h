/* Start Header ************************************************************************/
/*!
\file       SceneManager.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       March, 27th, 2025
\brief      

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "GameObjectManager.h"
#include "GameState.h"
#include <string>
#include <functional>
#include <unordered_map>

class LuaSystem;
class VisualLogicSystem;

class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void init(GameObjectManager& manager, LuaSystem& luaSystem, VisualLogicSystem& vlSystem);

    void loadScreen(GameObjectManager& manager, const std::string& sceneName);
    void loadScreenNoButton(GameObjectManager& manager, const std::string& sceneName);
    void changeState(GameState newState, const std::string& sceneName = "");

    GameState getCurrentState() const { return m_currentState; }
    void setCurrentState(GameState state) { m_currentState = state; }
    void setPreviousState(GameState state) { m_previousState = state; }
    void setStateBeforePause(GameState state) { m_stateBeforePause = state; }
    GameState getPreviousState() const { return m_previousState; }
    GameState getStateBeforePause() const { return m_stateBeforePause; }

    GameState sceneToState(const std::string& sceneName);

    void startGame();
    void stopGame();
    void restartLevel();
    void loadLevel(const std::string& levelSceneName);
    void resetAllObjectsToSpawn();

    std::function<void(GameObjectManager&)> m_onSceneLoadCallback;
    void setOnSceneLoadCallback(std::function<void(GameObjectManager&)> cb) {
        m_onSceneLoadCallback = cb;
    }
    void triggerSceneLoad(GameObjectManager& manager) {
        if (m_onSceneLoadCallback) m_onSceneLoadCallback(manager);
    }

    bool m_pendingRestart = false;
    bool m_pendingLevelLoad = false;
    std::string m_pendingLevelName = "";

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    GameState m_currentState = GameState::NONE;
    GameState m_previousState = GameState::MENU;
    GameState m_stateBeforePause = GameState::MENU;

    GameObjectManager* g_manager = nullptr;
    LuaSystem* g_luaSystem = nullptr;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    int level = 0;
    int max_level = 2;

    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    std::string m_currentLevel = "";
};