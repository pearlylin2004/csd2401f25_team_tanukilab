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
#include "SceneManager.h"
#include <string>
#include <unordered_map>
#include <functional> //for std::function to store button actions

class LuaSystem;
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

    //getter and setter for game state from scenemanager
    GameState getCurrentState()  const { return SceneManager::getInstance().getCurrentState(); }
    GameState getPreviousState() const { return SceneManager::getInstance().getPreviousState(); }
    GameState getStateBeforePause() const { return SceneManager::getInstance().getStateBeforePause(); }
    void setStateBeforePause(GameState s) { SceneManager::getInstance().setStateBeforePause(s); }
    void setCurrentState(GameState s) { SceneManager::getInstance().setCurrentState(s); }

	GameState sceneToState(const std::string& sceneName); //convert scene name to game state

    void resetAllObjectsToSpawn(); // helper for restart

    // for on scene load
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
    GUISystem() = default;
    ~GUISystem() = default;
    GUISystem(const GUISystem&) = delete; // prevent copying
    GUISystem& operator=(const GUISystem&) = delete; // prevent assignment

    void startGame(); //start game
    void stopGame(); //stop game	
    void updateVolumeDisplay(); //update volume % text on audio page

    GameObjectManager* g_manager = nullptr;
    LuaSystem* g_luaSystem = nullptr;
    VisualLogicSystem* g_visualLogicSystem = nullptr;

    int level = 0;

    std::unordered_map<std::string, Vector2D> m_spawnPositions;
    std::string m_currentLevel = "";

    float m_previousVolume = 100;
};