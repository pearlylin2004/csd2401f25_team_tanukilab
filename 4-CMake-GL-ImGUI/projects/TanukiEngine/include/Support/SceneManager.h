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

// ---------------------------------------------------------------------------
// SceneManager  (singleton)
// ---------------------------------------------------------------------------
// Owns everything to do with *loading scenes* and *tracking game state*.
// It does NOT know about buttons or input – that is ButtonSystem's job.
// ---------------------------------------------------------------------------
class SceneManager {
public:
    // -----------------------------------------------------------------------
    // Singleton
    // -----------------------------------------------------------------------
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    // -----------------------------------------------------------------------
    // Initialisation
    // -----------------------------------------------------------------------
    void init(GameObjectManager& manager, VisualLogicSystem& vlSystem);

    // -----------------------------------------------------------------------
    // Scene loading
    // -----------------------------------------------------------------------

    // Load a scene by file name (e.g. "menu_scene.json").
    // Stops all audio, clears the current scene, loads the new one, and
    // initialises resources.  After the call the caller should re-register
    // any buttons with ButtonSystem.
    void loadScene(const std::string& sceneName);

    // -----------------------------------------------------------------------
    // Level management  (deferred – applied at start of next frame)
    // -----------------------------------------------------------------------

    // Request a full level reload.  Applied at the start of the next update().
    void requestRestart();

    // Request loading a different level scene next frame.
    void requestLevelLoad(const std::string& levelSceneName);

    // Call once per frame from the main game loop (or GUISystem::update).
    // Flushes any pending restart / level-load requests.
    void update();

    // -----------------------------------------------------------------------
    // State management
    // -----------------------------------------------------------------------
    void changeState(GameState newState, const std::string& sceneName = "");

    GameState getCurrentState()    const { return m_currentState; }
    GameState getPreviousState()   const { return m_previousState; }
    GameState getStateBeforePause()const { return m_stateBeforePause; }

    void setCurrentState(GameState s) { m_currentState = s; }
    void setPreviousState(GameState s) { m_previousState = s; }
    void setStateBeforePause(GameState s) { m_stateBeforePause = s; }

    // Convert a scene filename to its corresponding GameState.
    GameState sceneToState(const std::string& sceneName) const;

    // Current level full path (used for restarts).
    const std::string& getCurrentLevel() const { return m_currentLevel; }
    void setCurrentLevel(const std::string& fullPath) { m_currentLevel = fullPath; }

    // -----------------------------------------------------------------------
    // Spawn-position helpers (for level restart)
    // -----------------------------------------------------------------------
    void captureSpawnPositions();
    void resetAllObjectsToSpawn();

private:
    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    void executeRestart();
    void executeLevelLoad(const std::string& levelSceneName);

    static std::string buildScenePath(const std::string& name);

    // -----------------------------------------------------------------------
    // Singleton boilerplate
    // -----------------------------------------------------------------------
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    // -----------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------
    GameObjectManager* g_manager = nullptr;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    GameState m_currentState = GameState::NONE;
    GameState m_previousState = GameState::MENU;
    GameState m_stateBeforePause = GameState::MENU;

    std::string m_currentLevel;  // full path to the active level scene

    // Spawn positions captured when a level first loads (used by restart).
    std::unordered_map<std::string, Vector2D> m_spawnPositions;

    // -----------------------------------------------------------------------
    // Pending (deferred) requests
    // -----------------------------------------------------------------------
    bool        m_pendingRestart = false;
    bool        m_pendingLevelLoad = false;
    std::string m_pendingLevelName;
};
