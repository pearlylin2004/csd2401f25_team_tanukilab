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
#include "ButtonAction.h"
#include <string>
#include <unordered_map>

class VisualLogicSystem; // forward declaration

class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void init(GameObjectManager& manager, VisualLogicSystem& vlSystem);
    void loadScene(const std::string& sceneName);
    void requestRestart();
    void requestLevelLoad(const std::string& levelSceneName);
    void update();
    void changeState(GameState newState, const std::string& sceneName = "");

    GameState getCurrentState()    const { return m_currentState; }
    GameState getPreviousState()   const { return m_previousState; }
    GameState getStateBeforePause()const { return m_stateBeforePause; }

    void setCurrentState(GameState s) { m_currentState = s; }
    void setPreviousState(GameState s) { m_previousState = s; }
    void setStateBeforePause(GameState s) { m_stateBeforePause = s; }
    const std::string& getPreviousSceneName() const { return m_previousSceneName; }

    GameState sceneToState(const std::string& sceneName) const;

    const std::string& getCurrentLevel() const { return m_currentLevel; }
    void setCurrentLevel(const std::string& fullPath) { m_currentLevel = fullPath; }
    void captureSpawnPositions();
    void resetAllObjectsToSpawn();

    void executeRestart();
    void executeLevelLoad(const std::string& levelSceneName);

    void pauseGame(const std::string& pauseSceneName = "pause_scene.json");
    void resumeGame();
    bool isPaused() const { return m_currentState == GameState::PAUSED; }
    std::string getLevelSceneName() const { return m_levelSceneName; }

    void setOnSceneLoadCallback(std::function<void(GameObjectManager&)> cb) {
        m_onSceneLoad = cb;
    }

    void requestNextLevel();
private:
    static std::string buildScenePath(const std::string& name);
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    GameObjectManager* g_manager = nullptr;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    GameState m_currentState = GameState::NONE;
    GameState m_previousState = GameState::MENU;
    GameState m_stateBeforePause = GameState::MENU;

    //for back
    std::string m_previousSceneName;
    std::string m_currentSceneName;

    std::string m_currentLevel;  //full path to the active level scene
    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    bool        m_pendingRestart = false;
    bool        m_pendingLevelLoad = false;
    std::string m_pendingLevelName;
    std::function<void(GameObjectManager&)> m_onSceneLoad;
    std::string m_levelSceneName; //only set when PLAYING state
    bool m_pendingIsWinScene = false;

    //next level
    std::vector<std::string> m_levelList;
    int m_currentLevelIndex = 0;
};
