/* Start Header ************************************************************************/
/*!
\file       GUISystem.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 5% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 50% of the file
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 5% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\par        cheeqing.tan@digipen.edu
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
#include "GameState.h"
#include "input.h"
#include "Component.h"
#include <string>
#include <unordered_map>
#include <functional> //for std::function to store button actions

class LuaSystem;

//enum for button actions
namespace std {
    template <>
    struct hash<ButtonAction> {
        size_t operator()(const ButtonAction& b) const noexcept {
            return std::hash<int>()(static_cast<int>(b));
        }
    };
}

class VisualLogicSystem; // forward decla that save my life

class GUISystem {
public:
    //singleton accessor
    static GUISystem& getInstance() {
        static GUISystem instance; //created on first call, destroyed automatically at program end
        return instance;
    }

    void init(GameObjectManager& manager, LuaSystem& luaSystem, VisualLogicSystem& vlSystem);
    void update(); //update menu system
    void loadScreen(GameObjectManager& manager, const std::string& sceneName); //load a screen
    void loadScreenNoButton(GameObjectManager& manager, const std::string& sceneName); //for no buttons
    void changeState(GameState newState, const std::string& sceneName = ""); //update of state

    GameState getCurrentState() const { return m_currentState; } //check current state
    void setCurrentState(GameState state) { m_currentState = state; } //set current state
    void setPreviousState(GameState state) { m_previousState = state; } //set previous state
    void setStateBeforePause(GameState state) { m_stateBeforePause = state; } //set state before pause

	GameState sceneToState(const std::string& sceneName); //convert scene name to game state

    void restartLevel(); // restart current level
	void loadLevel(const std::string& levelSceneName); // load next level
    void resetAllObjectsToSpawn(); // helper for restart

    // Replay viewer
    void startReplayViewer();

    void createButtons(/*GameObjectManager& manager*/); //create menu buttons

    // Level timer for leaderboard
    double getLevelTime() const { return m_levelTimer; }
    void tickLevelTimer(double dt);

    ButtonAction m_pendingAction = ButtonAction::NONE;
    bool m_pendingRestart = false;
    bool m_pendingLevelLoad = false;
    std::string m_pendingLevelName = "";

private:
    GUISystem() = default;
    ~GUISystem() = default;
    GUISystem(const GUISystem&) = delete; // prevent copying
    GUISystem& operator=(const GUISystem&) = delete; // prevent assignment

    void removeButtons(); //remove menu buttons
    void startGame(); //start game
    void stopGame(); //stop game	
    void updateVolumeDisplay(); //update volume % text on audio page
    void updateLeaderboardDisplay(); //populate leaderboard font objects with scores
    void loadReplayLevel(int levelIndex); // load a level in replay viewer mode

    bool isPointInButton(const Vector2D& point, GameObject* button);
    void createButtonActions(); //create button actions

    std::unordered_map<ButtonAction, GameObject*> m_buttons; //for storing buttons 
	std::unordered_map<ButtonAction, std::function<void()>> m_buttonActions; //map of button actions

    GameState m_currentState = GameState::NONE;
    GameState m_previousState = GameState::MENU;
    GameState m_stateBeforePause = GameState::MENU;
    GameObjectManager* g_manager = nullptr;
    LuaSystem* g_luaSystem = nullptr;

    int level = 0;
    int max_level = 2;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    std::string m_currentLevel = "";

    float m_previousVolume = 100;

    // Level timer for replay ghosts (resets each level)
    double m_levelTimer = 0.0;
    bool m_levelTimerRunning = false;

    // Total game timer for leaderboard (start to win, never resets between levels)
    double m_gameTimer = 0.0;
    bool m_gameTimerRunning = false;

    // Replay viewer state
    std::vector<std::string> m_replayLevels; // ordered list of levels to replay
    int m_replayLevelIndex = 0;
    bool m_replayViewerActive = false;
    std::string m_replayRunId;  // which run folder to replay from
    std::string m_lastRunId;    // the run ID of the most recently completed game
};