/* Start Header ************************************************************************/
/*!
\file       ButtonSystem.cpp
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

#include "ButtonSystem.h"
#include "SceneManager.h"
#include "Systems.h"      // UISystem, EditorManager
#include "input.h"        // InputHandler
#include "audio.h"        // AudioHandler, AudioComponent
#include "Component.h"    // Button, Transform, FontComponent
#include "GameState.h"
#include <iostream>
#include <unordered_map>

//init
void ButtonSystem::init(GameObjectManager& manager) {
    g_manager = &manager;
}

//refresh -- the old createButtons from GUI
void ButtonSystem::refresh() {
    clearButtons();
    if (!g_manager) return;

    std::vector<GameObject*> all;
    g_manager->getAllGameObjects(all);

    for (GameObject* obj : all) {
        if (!obj) continue;
        if (!obj->hasComponent<Button>() || !obj->hasComponent<Transform>()) continue;
        m_buttons.push_back(obj);
    }

    if (m_buttons.empty())
        std::cout << "[ButtonSystem] No buttons found in current scene.\n";
    else
        std::cout << "[ButtonSystem] Registered " << m_buttons.size() << " button(s).\n";

    //refresh volume text if we are on the audio page.
    if (SceneManager::getInstance().getCurrentState() == GameState::AUDIO)
        updateVolumeDisplay();
}

//update -- the old createButtonAction from GUI
void ButtonSystem::update() {
    if (!g_manager) return;
    if (UISystem::isShowUI()) return;
    if (EditorManager::isEditingMode()) return;

    Vector2D mouse = InputHandler::getWorldMousePosition();
    for (GameObject* obj : m_buttons) {
        Button* btn = obj->getComponent<Button>();
        if (!btn) continue;
        bool isOver = isPointInButton(mouse, obj);
        if (isOver && !btn->isHovered && !btn->onHoverChannel.empty()) {
            if (auto* audio = obj->getComponent<AudioComponent>()) {
                if (auto* ch = audio->getChannel(btn->onHoverChannel)) {
                    ch->isPendingPlay = true;
                }
            }
        }
        if (!isOver && btn->isHovered && !btn->onHoverChannel.empty()) {
            if (auto* audio = obj->getComponent<AudioComponent>()) {
                if (auto* ch = audio->getChannel(btn->onHoverChannel)) {
                    ch->isPendingStop = true;
                }
            }
        }
        btn->isHovered = isOver;
        if (isOver && InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
			std::cout << "[ButtonSystem] Button '" << obj->getObjectName() << "' clicked. Action: " << static_cast<int>(btn->actionName) << "\n";
			std::cout << "[ButtonSystem] Button scene path " << btn->scenePath << "'\n";
            // play click sound
            if (!btn->onClickChannel.empty()) {
                if (auto* audio = obj->getComponent<AudioComponent>()) {
                    if (auto* ch = audio->getChannel(btn->onClickChannel)) {
                        ch->isPendingPlay = true;
                    }
                }
            }
            handleAction(*btn);
            return;
        }
    }
}

//handle button action
void ButtonSystem::handleAction(const Button& btn) {
    switch (btn.actionName) {

    case ButtonAction::LOAD_SCENE:
        if (!btn.scenePath.empty()) {
            SceneManager::getInstance().requestLevelLoad(btn.scenePath);
        }
        break;

    case ButtonAction::EXIT_GAME:
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        break;

    case ButtonAction::MUSIC_VOL_UP: {
        auto& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() + 0.1f);
        updateVolumeDisplay();
        break;
    }

    case ButtonAction::MUSIC_VOL_DOWN: {
        auto& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() - 0.1f);
        updateVolumeDisplay();
        break;
    }
    case ButtonAction::BACK: {
        SceneManager::getInstance().requestLevelLoad(
            SceneManager::getInstance().getPreviousSceneName()
        );
        break;
    }
    case ButtonAction::MUTE: {
        auto& ah = AudioHandler::getInstance();
		m_previousVolume = ah.getMasterVolume();
        ah.setMasterVolume(0);
        updateVolumeDisplay();
        break;
    }
    case ButtonAction::UNMUTE: {
        auto& ah = AudioHandler::getInstance();
        ah.setMasterVolume(m_previousVolume);
        updateVolumeDisplay();
        break;
    }

    default:
        break;
    }
}

//volume display update
void ButtonSystem::updateVolumeDisplay() const {
    AudioHandler& ah = AudioHandler::getInstance();
    GameObject* volText = g_manager->getGameObject("music_vol_text");
    if (volText && volText->hasComponent<FontComponent>()) {
        int pct = static_cast<int>(ah.getMasterVolume() * 100.0f + 0.5f);
        volText->getComponent<FontComponent>()->word = std::to_string(pct) + "%";
    }
}

//clear button
void ButtonSystem::clearButtons() {
    m_buttons.clear();
}

//check if pointing in button
bool ButtonSystem::isPointInButton(const Vector2D& point, GameObject* button) const {
    if (!button || !button->hasComponent<Transform>()) return false;
    Transform* t = button->getComponent<Transform>();

    float hw = t->scaleX * 0.5f;
    float hh = t->scaleY * 0.5f;

    return (point.x >= t->x - hw && point.x <= t->x + hw &&
        point.y >= t->y - hh && point.y <= t->y + hh);
}
