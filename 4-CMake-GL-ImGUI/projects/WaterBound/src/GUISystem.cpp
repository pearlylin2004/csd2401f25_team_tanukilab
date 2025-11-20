/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 60% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 40% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\date       November, 7th, 2025
\brief      Handles initialization, updates, and GUI interactions such as
            creating/removing menu buttons and transitioning between
            MENU and PLAYING states.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include <iostream>
#include <fstream>

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

GUISystem::GUISystem() {
    std::cout << "[GUI] GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "[GUI] GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    manager.loadScene(runtimeScenePath("menu_scene.json"));
    // Assign background layer explicitly
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);
    for (GameObject* obj : allObjs) {
        const std::string& name = obj->getObjectName();
        if (name == "background") {
            manager.assignObjectToLayer(obj, 0); // layer 0 for background
        }
    }

	createButtons(manager); //create buttons from loaded scene

    g_manager = &manager;
    g_luaSystem = &luaSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;
}

//update function to check for button clicking
void GUISystem::update(GameObjectManager& manager) {
    // Only handle GUI interactions in menu state
    if (m_currentState != GameState::MENU) return;

    //get mouse position
    Vector2D worldMousePos = InputHandler::getWorldMousePosition();
    Vector2D worldPos = Vec_Set(worldMousePos.x, worldMousePos.y);

    if (!InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
		//when hover over button we can add hover effect here
    }

    else {
        //loop through all buttons in the map
        for (auto& pair : m_buttons) {
            const std::string& name = pair.first;
            GameObject* btn = pair.second;

            if (!btn) continue;

            if (isPointInButton(worldPos, btn)) {
                //handle button action dynamically based on name
                if (name == "play_button") {
                    startGame(manager);
                    return; //stop checking after a click
                }
                else if (name == "exit_button") {
                    stopGame();
                    return; //stop checking after a click
                }
                else if (name == "crontrol_button") {
					controlScreen(manager);
                    return; //stop checking after a click
                }
                //idk if we need more buttons
            }
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
        const std::string name = obj->getObjectName();
        //only consider objects that have both transform and render
        if (!obj->hasComponent<Transform>() || !obj->hasComponent<Render>()) continue;

		Render* r = obj->getComponent<Render>(); //get render

        //find if its a button
        bool isButton = r->hasTex || (name.find("_button") != std::string::npos);

        if (name == "background") {
            manager.assignObjectToLayer(obj, 2);
        }
        else if (isButton) {
            manager.assignObjectToLayer(obj, 0);
            m_buttons[name] = obj;
        }
        else {
			return; //not a button so skip
        }
    }
    m_buttonsCreated = !m_buttons.empty(); //buttons created if map is not empty
    std::cout << "[GUI] Menu buttons created successfully" << std::endl;
}

//removing of buttons 
void GUISystem::removeMenuButtons(GameObjectManager& manager) {
    //delete button objects
    for (auto& pair : m_buttons) {
		GameObject* btn = pair.second; //get button object
        if (btn) {
			manager.deleteGameObject(btn); //delete from manager
        }
    }
    m_buttons.clear(); //clear map
    m_buttonsCreated = false;
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "[GUI] Play button clicked - starting game" << std::endl;
    
    // Remove menu buttons
    removeMenuButtons(manager);

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

void GUISystem::controlScreen(GameObjectManager& manager) {
    std::cout << "[GUI] Control button clicked" << std::endl;

    // Remove menu buttons
    removeMenuButtons(manager);

    manager.loadScene(runtimeScenePath("control_scene.json"));
}