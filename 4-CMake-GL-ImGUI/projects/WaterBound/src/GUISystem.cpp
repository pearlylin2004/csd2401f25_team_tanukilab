/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 5% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 95% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\date       November, 7th, 2025
\brief      Handles initialization, updates, and GUI interactions such as
            creating/removing menu buttons and transitioning between
            MENU, CONTROl and PLAYING states.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include "CoreEngine.h"
#include <iostream>
#include <fstream>

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

/*GUISystem::GUISystem() :
    m_currentState(GameState::NONE),
    m_previousState(GameState::MENU),
    m_stateBeforePause(GameState::MENU),
    g_manager(nullptr),
    g_luaSystem(nullptr) {
    std::cout << "[GUI] GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "[GUI] GUISystem destructor called" << std::endl;
}*/

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    loadScreen(manager, "menu_scene.json");

    g_manager = &manager;
    g_luaSystem = &luaSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;
}

//update function to check for button clicking
void GUISystem::update() {
    if (EditorManager::isEditingMode()) return;
    if (UISystem::isShowUI()) return; // skip if UI is shown
    //if (m_buttons.empty()) return;

    Vector2D worldMousePos = InputHandler::getWorldMousePosition();

    if (m_currentState == GameState::PLAYING) {
        std::vector<GameObject*> allObjs;
        g_manager->getAllGameObjects(allObjs);
        if (allObjs.empty()) return; //check if its empty
        Transform* playerT = nullptr;
        Transform* winT = nullptr;
		Transform* fireT = nullptr;
        CollisionInfo* playerC = nullptr;
        CollisionInfo* winC = nullptr;
		CollisionInfo* fireC = nullptr;
        //iterate through all objects to find buttons
        for (GameObject* obj : allObjs) {
            if (obj->getObjectName() == "player") {
                playerT = obj->getComponent<Transform>();
                playerC = obj->getComponent<CollisionInfo>();
            }
            if (obj->getObjectName() == "to win") {
                winT = obj->getComponent<Transform>();
                winC = obj->getComponent<CollisionInfo>();
            }
            if (obj->getObjectName() == "fire_right") {
                fireT = obj->getComponent<Transform>();
                fireC = obj->getComponent<CollisionInfo>();
            }
        }
        if (!playerT || !playerC || !winT || !winC ||!fireT || !fireC) return;

        Vector2D playerPos{ playerT->x, playerT->y };
        Vector2D playerSize = playerC->colliderSize;
        Vector2D winPos{ winT->x, winT->y };
        Vector2D winSize = winC->colliderSize;
		Vector2D firePos{ fireT->x, fireT->y };
		Vector2D fireSize = fireC->colliderSize;

        //std::cout << "check: " << std::endl;
        //std::cout << playerPos.x << " " << playerPos.y << " " << std::endl;
        //std::cout << winPos.x << " " << winPos.y << " " << std::endl;

        //AABB collision check
        bool win_collided =
            (playerPos.x + playerSize.x / 2 >= winPos.x - winSize.x / 2) &&
            (playerPos.x - playerSize.x / 2 <= winPos.x + winSize.x / 2) &&
            (playerPos.y + playerSize.y / 2 >= winPos.y - winSize.y / 2) &&
            (playerPos.y - playerSize.y / 2 <= winPos.y + winSize.y / 2);

        bool lose_collided =
            (playerPos.x + playerSize.x / 2 >= firePos.x - fireSize.x / 2) &&
            (playerPos.x - playerSize.x / 2 <= firePos.x + fireSize.x / 2) &&
            (playerPos.y + playerSize.y / 2 >= firePos.y - fireSize.y / 2) &&
			(playerPos.y - playerSize.y / 2 <= firePos.y + fireSize.y / 2);


        if (win_collided) {
            //collision check for win condition
            if (level == max_level) {
                std::cout << "Player wins!" << std::endl;
                m_currentState = GameState::WIN;
                loadScreen(*g_manager, "win_scene.json");
            }
			//check collision to load next level
            else {
                std::cout << "Level Complete! Loading next level..." << std::endl;
                level++;
                m_currentState = GameState::PLAYING;
                loadScreenNoButton(*g_manager, "level0" + std::to_string(level) + ".json");
            }
        }
        else if (lose_collided) {
            //collision check for lose condition
            std::cout << "Player loses!" << std::endl;
            m_currentState = GameState::PLAYING;

            loadScreenNoButton(*g_manager,"level0" + std::to_string(level) + ".json");
        }
    }

    else if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        //iterate with iterator to safely remove invalid buttons
        for (auto it = m_buttons.begin(); it != m_buttons.end(); ) {
            GameObject* btn = it->second;
            if (isPointInButton(worldMousePos, btn)) {
                Button* btnComp = btn->getComponent<Button>(); //get button component
                if (btnComp) {
                    auto actionIt = m_buttonActions.find(btnComp->actionName); //find action by name
                    if (actionIt != m_buttonActions.end()) actionIt->second(); //execute action
                }
                return; //stop checking after a click
            }
            ++it;
        }
    }
}

//creation of buttons
void GUISystem::createButtons(GameObjectManager& manager) {
    //clear previous references
    removeButtons();

    //get all game objects from manager
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);

    if (allObjs.empty()) return; //check if its empty

    //iterate through all objects to find buttons
    for (GameObject* obj : allObjs) {
        if (!obj) continue; //safety check
        if (!obj->hasComponent<Button>() || !obj->hasComponent<Transform>()) continue; //check if it has button and transform component

        Button* btnComp = obj->getComponent<Button>(); //get button component
        m_buttons[btnComp->actionName] = obj; //store button in map with action name as key
    }

    if(m_buttons.empty()) {
        std::cout << "[GUI] No buttons found in the current scene." << std::endl;
	}
    else {
        createButtonActions(); //create button actions
        std::cout << "[GUI] Menu buttons created successfully" << std::endl;
    }   
}

//removing of buttons 
void GUISystem::removeButtons() {
    m_buttons.clear(); //clear map
}

//to update state
void GUISystem::changeState(GameState newState, const std::string& sceneName) {
    m_previousState = m_currentState;  // store previous
    m_currentState = newState;

    if (!sceneName.empty()) {
        loadScreen(*g_manager, sceneName);
    }
}

//create button actions
void GUISystem::createButtonActions() {
    m_buttonActions.clear();

    //play
    m_buttonActions[ButtonAction::START_GAME] = [this]() {
        changeState(GameState::PLAYING);
        startGame(*g_manager);
     };

    //exit confirmation
    m_buttonActions[ButtonAction::EXIT_GAME] = [this]() {
        changeState(GameState::EXITCONFIRMATION, "confirmation_scene.json");
     };

    //control screen
    m_buttonActions[ButtonAction::OPEN_CONTROL] = [this]() {
        changeState(GameState::CONTROL, "control_scene.json");
     };

    //back button
    m_buttonActions[ButtonAction::BACK] = [this]() {
        switch (m_previousState) {
        case GameState::MENU:
            changeState(GameState::MENU, "menu_scene.json");
            break;
        case GameState::CONTROL:
            changeState(GameState::CONTROL, "control_scene.json");
            break;
        case GameState::PAUSED:
            changeState(GameState::PAUSED, "pause_scene.json");
            break;
        case GameState::PLAYING:
            startGame(*g_manager); // resume the game
            break;
        case GameState::EXITCONFIRMATION:
            changeState(GameState::EXITCONFIRMATION, "confirmation_scene.json");
            break;
        default:
            changeState(GameState::MENU, "menu_scene.json");
            break;
        }
    };

    //menu button
    m_buttonActions[ButtonAction::MENU] = [this]() {
        changeState(GameState::MENU, "menu_scene.json");
    };

    //resume
    m_buttonActions[ButtonAction::RESUME_GAME] = [this]() {
        m_buttonActions[ButtonAction::BACK](); //uses logic from back
    };

    //exit confirmation screen yes/no
    m_buttonActions[ButtonAction::YES_EXIT] = [this]() {
        m_currentState = GameState::EXIT;
        stopGame();
    };
    m_buttonActions[ButtonAction::NO_EXIT] = [this]() {
        m_currentState = GameState::MENU;
        loadScreen(*g_manager, "menu_scene.json");
    };

    std::cout << "[GUI] Menu buttons created successfully" << std::endl;
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "[GUI] Starting game" << std::endl;

    //change state to playing
	m_currentState = GameState::PLAYING;
    level = 1;

    removeButtons(); //remove menu buttons

	loadScreenNoButton(*g_manager, "level01.json");

    std::vector<GameObject*> gameObjects;
    g_manager->getAllGameObjects(gameObjects);
    //iterate over all Lua scripts in the folder
    for (const auto& file : std::filesystem::directory_iterator("./assets/scripting")) {
        if (file.path().extension() != ".lua") continue; //have to be .lua files if not it will skip
        std::string filename = file.path().filename().string();

        //strip ".lua" to get object name
        std::string objectName = filename.substr(0, filename.size() - 4);

        GameObject* obj = g_manager->getGameObject(objectName);
        if (obj) {
            obj->addComponent<LuaScript>(file.path().string()); //add LuaScript component to object
            //load Lua script into Lua state
            if (!g_luaSystem->loadScriptForObject(file.path().string(), objectName)) {
                std::cerr << "[Lua] Failed to load script for " << objectName << std::endl;
            }
            else {
                std::cout << "[Lua] Loaded script " << filename << " for object " << objectName << std::endl;
            }
        }
    }
    std::cout << "[GUI] Game started - loaded json" << std::endl;
}

//stop game and quit
void GUISystem::stopGame() {
    std::cout << "[GUI] Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

//check if mouse is pointing the button
bool GUISystem::isPointInButton(const Vector2D& point, GameObject* button) {
    if (!button || !button->hasComponent<Transform>()) return false;

    Transform* transform = button->getComponent<Transform>();

    // Simple AABB collision check for button
    float halfWidth = transform->scaleX * 0.5f;
    float halfHeight = transform->scaleY * 0.5f;

    return (point.x >= transform->x - halfWidth &&
        point.x <= transform->x + halfWidth &&
        point.y >= transform->y - halfHeight &&
        point.y <= transform->y + halfHeight);
}

//buttons
void GUISystem::loadScreen(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Loading screen: " << sceneName << "\n";

    removeButtons(); // clear references
    manager.clearScene(); //delete al scene object

    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources();

    createButtons(manager);
}

//no buttons
void GUISystem::loadScreenNoButton(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Loading screen: " << sceneName << "\n";

    manager.clearScene(); //delete al scene object
    manager.init();
    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources();
}


//helper function to convert scene name to game state
GameState GUISystem::sceneToState(const std::string& sceneName) {
    if (sceneName == "menu_scene.json") {
        return GameState::MENU;
    }
    if (sceneName == "control_scene.json") {
        return GameState::CONTROL;
    }
    if (sceneName == "level01.json" || sceneName == "level02.json") {
		//if (!m_buttons.empty()) removeButtons();
        return GameState::PLAYING;
    }
    if (sceneName == "pause_scene.json") {
        return GameState::PAUSED;
    }
    if (sceneName == "confirmation_scene.json") {
        return GameState::EXITCONFIRMATION;
    }
    return GameState::MENU; //fallback
}
