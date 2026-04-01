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
    void update();

    void loadScene(const std::string& sceneName);
    void restartScene();
    void pauseScene(); // pause (no scene load)
    void resumeScene();

    bool isPaused() const { return m_paused; }
    const std::string& getCurrentSceneName() const { return m_currentSceneName; }
    const std::string& getPreviousSceneName() const { return m_previousSceneName; }

    void setOnSceneLoadCallback(std::function<void(GameObjectManager&)> callb) {
        m_onSceneLoad = callb;
    }

    // pending requests (called from Lua/game code, executed next frame)
    void requestRestart();
    void requestLoadScene(const std::string& levelSceneName);

    void triggerSceneLoad(GameObjectManager& manager);
    void resetAndInitVisualLogic(GameObjectManager& manager);

private:
    static std::string buildScenePath(const std::string& name);
    void executeRestart();
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
    std::string m_previousSceneName;
    std::string m_currentSceneFullPath; // for restart

    bool m_paused = false;
    bool m_pendingRestart = false;
    bool m_pendingLoad = false;
    std::string m_pendingLoadName;

    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    std::function<void(GameObjectManager&)> m_onSceneLoad;
};
