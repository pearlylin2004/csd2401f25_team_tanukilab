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

/**
 * @class SceneManager
 * @brief Singleton that manages scene loading, transitions, pause/resume, and restart.
 *
 * Handles all scene lifecycle operations including:
 * - Loading scenes from file with optional history tracking for back navigation
 * - Pause/resume by snapshotting the current scene state to memory
 * - Restart by reloading the current scene and resetting object spawn positions
 * - A pending request system so scene changes happen at a safe point in the frame
 */
class SceneManager {
public:
    /** @brief Returns the singleton instance. */
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    /** @brief Initializes the SceneManager with the GameObjectManager and VisualLogicSystem references. */
    void init(GameObjectManager& manager, VisualLogicSystem& vlSystem);

    /**
     * @brief Processes any pending scene requests (load, restart, resume).
     * Should be called once per frame. Deferred execution ensures scene
     * changes happen at the end of the update cycle, not mid-frame.
     */
    void update();

    /** @brief Immediately loads a scene by name. Prefer requestLoadScene for in-game use. */
    void loadScene(const std::string& sceneName);

    /** @brief Requests a restart of the current scene, resetting all objects to spawn positions. */
    void restartScene();

    /** @brief Marks the scene as paused without loading a new scene. */
    void pauseScene();

    /** @brief Marks the scene as resumed without loading a new scene. */
    void resumeScene();

    /**
     * @brief Navigates to the previous scene in the history stack.
     * If currently paused and the previous scene is the paused-from scene,
     * triggers a resume (restores snapshot) instead of a fresh load.
     */
    void back();

    /** @brief Returns true if the scene is currently in a paused state. */
    bool isPaused() const { return m_isPaused; }

    /** @brief Returns the name of the currently loaded scene. */
    const std::string& getCurrentSceneName() const { return m_currentSceneName; }

    /** @brief Returns the name of the scene at the top of the history stack, or empty if none. */
    const std::string& getPreviousSceneName() const {
        static const std::string empty;
        return m_sceneHistory.empty() ? empty : m_sceneHistory.top();
    }

    /** @brief Removes the top entry from the scene history stack. */
    void popHistory();

    /**
     * @brief Registers a callback to be invoked whenever a scene finishes loading.
     * Used by CoreEngine to run GameScript::onSceneLoad after each scene transition.
     */
    void setOnSceneLoadCallback(std::function<void(GameObjectManager&)> callb) {
        m_onSceneLoad = std::move(callb);
    }

    // pending requests (called from Lua/game code, executed next frame)
    /**
     * @brief Requests a deferred scene load, executed next frame via update().
     * @param name Scene filename to load.
     * @param pushHistory If true, pushes the current scene onto the history stack.
     */
    void requestRestart();

    /** @brief Queues a scene load to be executed on the next update() call. */
    void requestLoadScene(const std::string& name, bool pushHistory = true);

    /**
     * @brief Snapshots the current scene to memory and loads the pause scene.
     * The snapshot is restored when back() triggers a resume.
     * @param pauseSceneName The scene to load as the pause menu.
     */
    void requestPause(const std::string& pauseSceneName);

    /** @brief Manually fires the scene load callback on the given manager. Used during editor play. */
    void triggerSceneLoad(GameObjectManager& manager);

    /** @brief Clears and re-initializes the VisualLogic system for the given manager. */
    void resetAndInitVisualLogic(GameObjectManager& manager);

    /** @brief Clears all pause state (isPaused, snapshot, pausedFromScene). */
    void clearPauseState();

private:
    /** @brief Builds the full runtime path for a scene file by name. */
    static std::string buildScenePath(const std::string& name);

    /** @brief Executes a scene restart — reloads from disk and resets spawn positions. */
    void executeRestart();

    /** @brief Restores the scene from the pause snapshot and resumes gameplay. */
    void executeResume();

    /** @brief Performs the actual scene load — clears current scene, loads from file, reinitializes. */
    void executeLoad(const std::string& levelSceneName);

    /** @brief Captures each object's current position as its spawn position for restart. */
    void captureSpawnPositions();

    /** @brief Resets all objects back to their captured spawn positions and clears runtime state. */
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
