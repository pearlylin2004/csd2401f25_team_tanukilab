/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 90% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
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

#define RUNTIME_SCENE_DIR_R "./assets/Scene"

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

GUISystem::GUISystem() {
    std::cout << "GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem) {
    std::cout << "GUISystem::init started" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    manager.loadScene(runtimeScenePath("menu_scene.json"));

    // Get button data from the manager
    if (manager.hasButtonData()) {
        m_buttonData = manager.getButtonData();
        m_buttonDataLoaded = true;
        std::cout << "Successfully loaded " << m_buttonData.size() << " button(s) from GameObjectManager" << std::endl;
    }
    else {
        std::cout << "No button data found in GameObjectManager" << std::endl;
        m_buttonDataLoaded = false;
    }

    // Create the actual button game objects
    createButtons(manager); //creating buttons from json
    g_manager = &manager;
    g_luaSystem = &luaSystem;
    std::cout << "GUISystem::init completed successfully" << std::endl;
}

void GUISystem::update(GameObjectManager& manager) {
    // Only handle GUI interactions in menu state
    if (m_currentState == GameState::MENU) {
        // Check for button clicks
        if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
            // Get mouse position as Vector2D
            Vector2D mousePos = InputHandler::getMousePosition();

            // Convert screen coordinates to world coordinates
            Vector2D worldMousePos = InputHandler::getWorldMousePosition();
            glm::vec2 worldPos = glm::vec2(worldMousePos.x, worldMousePos.y);

            // Check play button click
            if (m_playButton && isPointInButton(worldPos, m_playButton)) {
                startGame(manager);
                return;
            }

            // Check stop button click  
            if (m_stopButton && isPointInButton(worldPos, m_stopButton)) {
                stopGame(manager);
                return;
            }
        }
    }
}

void GUISystem::createButtons(GameObjectManager& manager) {
    //check if json loaded
    if (!m_buttonDataLoaded) {
        std::cout << "No JSON data loaded, cannot create GUI objects" << std::endl;
        return;
    }
    
    /*if (m_buttonsCreated) {
        std::cout << "Buttons already created, returning early" << std::endl;
        return;
    }*/

    //clear previous references
    m_playButton = nullptr;
    m_stopButton = nullptr;
    m_buttonsCreated = false;

	//get all game objects from manager
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);

	//loop through all objects to find buttons
    for (GameObject* obj : allObjs)
    {
        if (!obj) continue;

		//get object name
        const std::string name = obj->getObjectName();
        //only consider objects that have both transform and render
        if (!obj->hasComponent<Transform>() || !obj->hasComponent<Render>()) continue;

		Transform* t = obj->getComponent<Transform>(); // get transform
		Render* r = obj->getComponent<Render>(); // get render

        //find if its a button
        bool isButton = r->hasTex || (name.find("_button") != std::string::npos);

		if (!isButton) continue; //skip non buttons

        //sssign object to layer 2
        manager.assignObjectToLayer(obj, 2);

        //register special buttons so existing click logic works
        if (name == "play_button") {
            m_playButton = obj;
            std::cout << "[GUI] Registered play_button\n";
        }
        else if (name == "exit_button") {
            m_stopButton = obj;
            std::cout << "[GUI] Registered exit button: " << name << "\n";
        }

        std::cout << "[GUI] Created GUI button object: " << name
            << " pos=(" << t->x << "," << t->y << "," << t->z << ")"
            << " scale=(" << t->scaleX << "," << t->scaleY << ")\n";
    }

    /*// Create Play Button
    m_playButton = manager.createGameObject("play_button");
    if (!m_playButton) {
        std::cout << "ERROR: Failed to create play button!" << std::endl;
        return;
    }

    Render* playRender = m_playButton->addComponent<Render>();
    playRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    playRender->hasTex = false;

    Transform* playTransform = m_playButton->addComponent<Transform>();

    // Use JSON data if available
    if (m_buttonDataLoaded && m_buttonData.find("play_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["play_button"];
        playTransform->x = data.position.x;
        playTransform->y = data.position.y;
        playTransform->z = data.position.z;
        playTransform->scaleX = data.scale.x;
        playTransform->scaleY = data.scale.y;
        playTransform->scaleZ = data.scale.z;
        std::cout << "Play button created with JSON data: pos("
            << data.position.x << ", " << data.position.y << ", " << data.position.z
            << ") scale(" << data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
    }
    else {
        // Fallback to default values
        playTransform->x = -6.0f;
        playTransform->y = 0.0f;
        playTransform->z = 0.1f;
        playTransform->scaleX = 6.0f;
        playTransform->scaleY = 1.0f;
        playTransform->scaleZ = 1.0f;
        std::cout << "Play button created with default data (no JSON data found)" << std::endl;
    }

    manager.assignObjectToLayer(m_playButton, 2);
    
    // Create Stop Button
    m_stopButton = manager.createGameObject("stop_button");
    if (!m_stopButton) {
        std::cout << "ERROR: Failed to create stop button!" << std::endl;
        return;
    }

    Render* stopRender = m_stopButton->addComponent<Render>();
    stopRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    stopRender->hasTex = false;

    Transform* stopTransform = m_stopButton->addComponent<Transform>();

    // Use JSON data if available
    if (m_buttonDataLoaded && m_buttonData.find("stop_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["stop_button"];
        stopTransform->x = data.position.x;
        stopTransform->y = data.position.y;
        stopTransform->z = data.position.z;
        stopTransform->scaleX = data.scale.x;
        stopTransform->scaleY = data.scale.y;
        stopTransform->scaleZ = data.scale.z;
        std::cout << "Stop button created with JSON data: pos("
            << data.position.x << ", " << data.position.y << ", " << data.position.z
            << ") scale(" << data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
    }
    else {
        // Fallback to default values
        stopTransform->x = 3.0f;
        stopTransform->y = 0.0f;
        stopTransform->z = 0.1f;
        stopTransform->scaleX = 6.0f;
        stopTransform->scaleY = 1.0f;
        stopTransform->scaleZ = 1.0f;
        std::cout << "Stop button created with default data (no JSON data found)" << std::endl;
    }

    manager.assignObjectToLayer(m_stopButton, 2);*/

    m_buttonsCreated = true;
    m_currentState = GameState::MENU;
    //std::cout << "Menu buttons created successfully" << std::endl;
}

void GUISystem::removeMenuButtons(GameObjectManager& manager) {
    if (m_playButton) {
        manager.deleteGameObject(m_playButton);
        m_playButton = nullptr;
    }
    if (m_stopButton) {
        manager.deleteGameObject(m_stopButton);
        m_stopButton = nullptr;
    }
    m_buttonsCreated = false;
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "Play button clicked - starting game" << std::endl;
    
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
    std::cout << "Game started - loaded json" << std::endl;
}

void GUISystem::stopGame(GameObjectManager& manager) {
    std::cout << "Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

bool GUISystem::isPointInButton(const glm::vec2& point, GameObject* button) {
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