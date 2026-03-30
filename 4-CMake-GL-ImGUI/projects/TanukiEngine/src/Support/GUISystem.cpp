/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 5% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 55% of the file
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 10% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\par        cheeqing.tan@digipen.edu
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

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem, VisualLogicSystem& vlSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;
    g_manager = &manager;
    g_luaSystem = &luaSystem;
    g_visualLogicSystem = &vlSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;
}

//update function to check for button clicking
void GUISystem::update() {
	ButtonSystem::getInstance().update(); //check for button clicking and handle actions
}

void GUISystem::startGame() {
    std::cout << "[GUI] Starting game" << std::endl;
    level = 1;

    // Delegate to SceneManager
    SceneManager::getInstance().requestLevelLoad("level01.json");

    // Lua script loading still lives here — runs after scene is loaded via callback
    SceneManager::getInstance().setOnSceneLoadCallback([this](GameObjectManager& mgr) {
        for (const auto& file : std::filesystem::directory_iterator("./assets/scripting")) {
            if (file.path().extension() != ".lua") continue;
            std::string filename = file.path().filename().string();
            std::string objectName = filename.substr(0, filename.size() - 4);
            GameObject* obj = mgr.getGameObject(objectName);
            if (obj) {
                obj->addComponent<LuaScript>(file.path().string());
                if (!g_luaSystem->loadScriptForObject(file.path().string(), objectName))
                    std::cerr << "[Lua] Failed to load script for " << objectName << std::endl;
                else
                    std::cout << "[Lua] Loaded script " << filename << " for object " << objectName << std::endl;
            }
        }
        // Clear callback after use so it doesn't fire on subsequent loads
        SceneManager::getInstance().setOnSceneLoadCallback(nullptr);
        });

    std::cout << "[GUI] Game start requested" << std::endl;
}

//stop game and quit
void GUISystem::stopGame() {
    std::cout << "[GUI] Exit button clicked - quitting application" << std::endl;

    //close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

GameState GUISystem::sceneToState(const std::string& sceneName) {
    return SceneManager::getInstance().sceneToState(sceneName);
}