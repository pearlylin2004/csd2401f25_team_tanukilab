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

    /* -------- Gamepad Menu Navigation -------- */
    auto& buttonSystem = ButtonSystem::getInstance();
    const auto& rawButtons = buttonSystem.getButtons();
    if (InputHandler::isGamepadConnected() && !rawButtons.empty()) {
        // sort buttons top to bottom by Y position
        std::vector<GameObject*> sortedButtons(rawButtons.begin(), rawButtons.end());
        std::sort(sortedButtons.begin(), sortedButtons.end(), [](auto& a, auto& b) {
            Transform* ta = a->getComponent<Transform>();
            Transform* tb = b->getComponent<Transform>();
            if (!ta || !tb) return false;
            return ta->y > tb->y; // higher Y go first
            });

        int btnCount = (int)sortedButtons.size();

        // D-pad to cycle selection (-1 = nothing selected yet)
        if (InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) {
            if (m_selectedButtonIdx == -1)
                m_selectedButtonIdx = 0; // first press goes to first button
            else
                m_selectedButtonIdx = (m_selectedButtonIdx + 1) % btnCount;
        }
        if (InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_DPAD_UP)) {
            if (m_selectedButtonIdx == -1)
                m_selectedButtonIdx = 0; // first press goes to first button
            else
                m_selectedButtonIdx = (m_selectedButtonIdx - 1 + btnCount) % btnCount;
        }

        // update hover state for all buttons
        if (m_selectedButtonIdx != -1) {
            for (int i = 0; i < btnCount; ++i) {
                GameObject* btnObj = sortedButtons[i];
                Button* btnComp = btnObj->getComponent<Button>();
                Render* render = btnObj->getComponent<Render>();
                if (!btnComp) continue;

                bool isSelected = (m_selectedButtonIdx != -1 && i == m_selectedButtonIdx);

                // visual tint for gamepad selection
                if (render) {
                    if (isSelected)
                        render->clr = glm::vec4(1.f, 1.f, 0.f, 1.f); // yellow tint
                    else
                        render->clr = glm::vec4(1.f, 1.f, 1.f, 1.f); // restore white
                }

                // trigger hover sound
                if (isSelected && !btnComp->isHovered && !btnComp->onHoverChannel.empty()) {
                    AudioComponent* audio = btnObj->getComponent<AudioComponent>();
                    if (audio) {
                        AudioChannel* ch = audio->getChannel(btnComp->onHoverChannel);
                        if (ch) ch->isPendingPlay = true;
                    }
                }

                // trigger hover exit sound
                if (!isSelected && btnComp->isHovered && !btnComp->onHoverChannel.empty()) {
                    AudioComponent* audio = btnObj->getComponent<AudioComponent>();
                    if (audio) {
                        AudioChannel* ch = audio->getChannel(btnComp->onHoverChannel);
                        if (ch) ch->isPendingStop = true;
                    }
                }

                btnComp->isHovered = isSelected;
            }
        }

        // Cross button (X) to confirm
        if (m_selectedButtonIdx != -1 && InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_A)) {
            GameObject* btnObj = sortedButtons[m_selectedButtonIdx];
            Button* btnComp = btnObj->getComponent<Button>();
            if (btnComp) {
                // play click sound
                if (!btnComp->onClickChannel.empty()) {
                    AudioComponent* audio = btnObj->getComponent<AudioComponent>();
                    if (audio) {
                        AudioChannel* ch = audio->getChannel(btnComp->onClickChannel);
                        if (ch) ch->isPendingPlay = true;
                    }
                }
                // execute action
                ButtonSystem::getInstance().setPendingAction(btnComp->actionName);
            }
        }
    }
}
void GUISystem::triggerSceneLoad(GameObjectManager& manager) {
    if (m_onSceneLoadCallback) m_onSceneLoadCallback(manager);
}

void GUISystem::resetAndInitVisualLogic(GameObjectManager& manager) {
    if (g_visualLogicSystem) {
        VisualLC::globalFloatVar.clear();
        g_visualLogicSystem->initAll(manager);
    }
}

void GUISystem::startGame() {
    std::cout << "[GUI] Starting game" << std::endl;
    level = 1;

    // Delegate to SceneManager
    SceneManager::getInstance().requestLevelLoad("Game_Level_1.json");

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
