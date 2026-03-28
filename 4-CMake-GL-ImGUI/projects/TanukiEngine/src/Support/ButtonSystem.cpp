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

//refresh
void ButtonSystem::refresh() {
    clearButtons();
    if (!g_manager) return;

    std::vector<GameObject*> all;
    g_manager->getAllGameObjects(all);

    for (GameObject* obj : all) {
        if (!obj) continue;
        if (!obj->hasComponent<Button>() || !obj->hasComponent<Transform>()) continue;

        Button* btn = obj->getComponent<Button>();
        m_buttons[btn->actionName] = obj;
    }

    if (m_buttons.empty())
        std::cout << "[ButtonSystem] No buttons found in current scene.\n";
    else
        std::cout << "[ButtonSystem] Registered " << m_buttons.size() << " button(s).\n";

    // Refresh volume text if we are on the audio page.
    if (SceneManager::getInstance().getCurrentState() == GameState::AUDIO)
        updateVolumeDisplay();
}

//update


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