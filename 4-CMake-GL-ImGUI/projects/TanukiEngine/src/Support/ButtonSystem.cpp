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

    // no need to broadcast if no buttons found, not all scene need to have buttons
    //if (m_buttons.empty())
    //    std::cout << "[ButtonSystem] No buttons found in current scene.\n";
    //else
    //    std::cout << "[ButtonSystem] Registered " << m_buttons.size() << " button(s).\n";

    ////refresh volume text if we are on the audio page.
    //if (SceneManager::getInstance().getCurrentState() == GameState::AUDIO)
    //    updateVolumeDisplay();
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

    // gamepad navigation
    if (!InputHandler::isGamepadConnected() || m_buttons.empty()) return;

    // sort top to bottom
    std::vector<GameObject*> sorted(m_buttons.begin(), m_buttons.end());
    std::sort(sorted.begin(), sorted.end(), [](GameObject* a, GameObject* b) {
        Transform* ta = a->getComponent<Transform>();
        Transform* tb = b->getComponent<Transform>();
        if (!ta || !tb) return false;
        return ta->y > tb->y;
        });

    int count = (int)sorted.size();

    if (InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_DPAD_DOWN))
        m_selectedButtonIdx = (m_selectedButtonIdx == -1) ? 0 : (m_selectedButtonIdx + 1) % count;

    if (InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_DPAD_UP))
        m_selectedButtonIdx = (m_selectedButtonIdx == -1) ? 0 : (m_selectedButtonIdx - 1 + count) % count;

    // reset selection when scene changes
    if (m_buttons.size() != m_lastButtonCount) {
        m_selectedButtonIdx = -1;
        m_lastButtonCount = m_buttons.size();
    }

    for (int i = 0; i < count; ++i) {
        GameObject* obj = sorted[i];
        Button* btn = obj->getComponent<Button>();
        Render* render = obj->getComponent<Render>();
        if (!btn) continue;

        bool isSelected = (i == m_selectedButtonIdx);

        if (render)
            render->clr = isSelected ? glm::vec4(1.f, 1.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);

        if (isSelected && !btn->isHovered && !btn->onHoverChannel.empty())
            if (auto* a = obj->getComponent<AudioComponent>())
                if (auto* ch = a->getChannel(btn->onHoverChannel))
                    ch->isPendingPlay = true;

        if (!isSelected && btn->isHovered && !btn->onHoverChannel.empty())
            if (auto* a = obj->getComponent<AudioComponent>())
                if (auto* ch = a->getChannel(btn->onHoverChannel))
                    ch->isPendingStop = true;

        btn->isHovered = isSelected;
    }

    if (m_selectedButtonIdx != -1 && InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_A)) {
        GameObject* obj = sorted[m_selectedButtonIdx];
        Button* btn = obj->getComponent<Button>();
        if (btn) {
            if (!btn->onClickChannel.empty())
                if (auto* a = obj->getComponent<AudioComponent>())
                    if (auto* ch = a->getChannel(btn->onClickChannel))
                        ch->isPendingPlay = true;
            handleAction(*btn);
        }
    }
}

//handle button action
void ButtonSystem::handleAction(const Button& btn) {
    switch (btn.actionName) {

    case ButtonAction::LOAD_SCENE:
        if (!btn.scenePath.empty()) {
            SceneManager::getInstance().requestLoadScene(btn.scenePath);
        }
        break;

    case ButtonAction::RESTART:
        SceneManager::getInstance().requestRestart();
        break;

    case ButtonAction::RESUME_GAME:
        SceneManager::getInstance().resumeScene();
        // load the level scene again if needed
        if (!SceneManager::getInstance().getCurrentSceneName().empty())
            SceneManager::getInstance().requestLoadScene(
                SceneManager::getInstance().getCurrentSceneName());
        break;

    case ButtonAction::BACK:
        if (!SceneManager::getInstance().getPreviousSceneName().empty())
            SceneManager::getInstance().requestLoadScene(
                SceneManager::getInstance().getPreviousSceneName());
        break;

    case ButtonAction::EXIT_GAME:
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
        break;

    case ButtonAction::MUSIC_VOL_UP: {
        auto& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() + 0.1f);
        //updateVolumeDisplay();
        break;
    }

    case ButtonAction::MUSIC_VOL_DOWN: {
        auto& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() - 0.1f);
        //updateVolumeDisplay();
        break;
    }

    case ButtonAction::MUTE: {
        auto& ah = AudioHandler::getInstance();
        m_previousVolume = ah.getMasterVolume();
        ah.setMasterVolume(0.f);
        //updateVolumeDisplay();
        break;
    }

    case ButtonAction::UNMUTE: {
        AudioHandler::getInstance().setMasterVolume(m_previousVolume);
        //updateVolumeDisplay();
        break;
    }

    default:
        break;
    }
}

//volume display update
//void ButtonSystem::updateVolumeDisplay() const {
//    AudioHandler& ah = AudioHandler::getInstance();
//    GameObject* volText = g_manager->getGameObject("music_vol_text");
//
//    if (volText && volText->hasComponent<FontComponent>()) {
//        int pct = static_cast<int>(ah.getMasterVolume() * 100.0f + 0.5f);
//        volText->getComponent<FontComponent>()->word = std::to_string(pct) + "%";
//    }
//}

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
