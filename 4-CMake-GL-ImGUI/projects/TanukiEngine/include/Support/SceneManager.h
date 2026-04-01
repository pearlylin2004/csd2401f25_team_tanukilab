/* Start Header ************************************************************************/
/*!
\file       SceneManager.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       March, 27th, 2025
\brief      Handle restart/load/pause/resume of scenes, and scene history for back button.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include <stack>
#include "GameObjectManager.h"
#include "ButtonAction.h"

class VisualLogicSystem; // forward declaration

class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void init(GameObjectManager& manager, VisualLogicSystem& vlSystem);
    void update();

    void loadScene(const std::string& sceneName);
    void restartScene();
    void pauseScene(); // pause (no scene load)
    void resumeScene();
    void back();

    bool isPaused() const { return m_isPaused; }
    const std::string& getCurrentSceneName() const { return m_currentSceneName; }
    const std::string& getPreviousSceneName() const {
        static const std::string empty;
        return m_sceneHistory.empty() ? empty : m_sceneHistory.top();
    }
    void popHistory();

    void setOnSceneLoadCallback(std::function<void(GameObjectManager&)> callb) {
        m_onSceneLoad = std::move(callb);
    }

    // pending requests (called from Lua/game code, executed next frame)
    void requestRestart();
    void requestLoadScene(const std::string& name, bool pushHistory = true);
    void requestPause(const std::string& pauseSceneName);

    void triggerSceneLoad(GameObjectManager& manager);
    void resetAndInitVisualLogic(GameObjectManager& manager);

private:
    static std::string buildScenePath(const std::string& name);
    void executeRestart();
    void executeResume();
    void executeLoad(const std::string& levelSceneName);
    void captureSpawnPositions();
    void resetAllObjectsToSpawn();

    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    GameObjectManager* g_manager = nullptr;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    std::string m_currentSceneName;
    std::string m_currentSceneFullPath; // for restart

    // pause
    std::string m_pausedFromScene;
    bool m_isPaused = false;
    bool m_pendingResume = false;
    std::string m_pauseSnapshot;

    // previous
    std::stack<std::string> m_sceneHistory;
    bool m_pendingPushHistory = true;

    bool m_pendingRestart = false;
    bool m_pendingLoad = false;
    std::string m_pendingLoadName;

    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    std::function<void(GameObjectManager&)> m_onSceneLoad;
};
