/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 10% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 90% of the file
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

GUISystem::GUISystem() :
    m_currentState(GameState::MENU), 
    m_previousState(GameState::MENU),
    m_stateBeforePause(GameState::MENU),
    g_manager(nullptr),
    g_luaSystem(nullptr) {
    std::cout << "[GUI] GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "[GUI] GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    loadScreen(manager, "menu_scene.json");

    g_manager = &manager;
    g_luaSystem = &luaSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;
}

//update function to check for button clicking
void GUISystem::update(GameObjectManager& manager) {
    if (UISystem::isShowUI()) return; //skip if UI is shown
    //get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
		//when hover over button we can add hover effect here
        return;
    }

	//loop through all buttons in the map
    for (auto& [_, btn] : m_buttons) {
		if (!btn || !btn->hasComponent<ButtonComponent>()) continue; //skip if no button component

		//check if mouse is in button
        if (isPointInButton(worldMousePos, btn)) {
			GameAction action = btn->getComponent<ButtonComponent>()->action; //get button action
			handleAction(action, manager); //handle button action
            return;
        }
    }
}

//creation of buttons
//can be use for creating buttons in game also
void GUISystem::createButtons(GameObjectManager& manager) {
    //clear previous references
    m_buttons.clear();

	//get all game objects from manager
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);

    if (allObjs.empty()) return; //check if its empty

	//loop through all objects to find buttons
    for (GameObject* obj : allObjs){
		//get object name
        //const std::string name = obj->getObjectName();
        //only consider objects that have both transform and render
        if (!obj->hasComponent<Transform>()) continue;

		//only consider objects that have button component
        if (!obj->hasComponent<ButtonComponent>()) {
            manager.assignObjectToLayer(obj, 0);
            continue;
        }

        //layering
        if (obj->hasComponent<FontComponent>()) {
            manager.assignObjectToLayer(obj, 1);
        }
        else {
            manager.assignObjectToLayer(obj, 2);
        }
        m_buttons[obj->getObjectName()] = obj;
    }
    std::cout << "[GUI] Menu buttons created successfully" << std::endl;
}

//removing of buttons 
void GUISystem::removeButtons(GameObjectManager& manager) {
    //delete button objects
    for (auto& pair : m_buttons) {
		GameObject* btn = pair.second; //get button object
        if (btn) {
			manager.deleteGameObject(btn); //delete from manager
        }
    }
    m_buttons.clear(); //clear map
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "[GUI] Play button clicked - starting game" << std::endl;
    
    // Remove menu buttons
    removeButtons(manager);

    // Load the game scene
    manager.init();
    manager.initializeSceneResources();

    // Change game state
    m_currentState = GameState::PLAYING;

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

//load a specific screen using the filename
void GUISystem::loadScreen(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Load Screen: " << sceneName << std::endl;
    removeButtons(manager);
    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources(); //to get resources loaded
    createButtons(manager); //loading scene
    std::cout << "[GUI] Load Screen created successfully!" << std::endl;
}

void GUISystem::handleAction(GameAction action, GameObjectManager& manager) {
    switch (action) {

    case GameAction::PLAY:
        m_stateBeforePause = GameState::MENU;
        m_currentState = GameState::PLAYING;
        startGame(manager);
        break;

    case GameAction::EXIT_GAME:
        m_currentState = GameState::EXITCONFIRMATION;
        loadScreen(manager, "confirmation_scene.json");
        break;

    case GameAction::OPEN_CONTROL:
        m_previousState = m_currentState;
        m_currentState = GameState::CONTROL;
        loadScreen(manager, "control_scene.json");
        break;

    case GameAction::BACK_TO_PREVIOUS:
        m_currentState = m_previousState;
        loadScreen(manager,
            m_previousState == GameState::PAUSED ?
            "pause_scene.json" : "menu_scene.json");
        break;

    case GameAction::RESUME_GAME:
        m_currentState = m_stateBeforePause;
        break;

    case GameAction::RETURN_TO_MENU:
        m_currentState = GameState::MENU;
        loadScreen(manager, "menu_scene.json");
        break;

    case GameAction::QUIT:
        stopGame();
        break;

    default:
        break;
    }
}

