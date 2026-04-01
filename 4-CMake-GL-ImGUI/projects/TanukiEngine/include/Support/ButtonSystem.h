/* Start Header ************************************************************************/
/*!
\file       ButtonSystem.h
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
#pragma once
#include "GameObjectManager.h"
#include "ButtonAction.h"
#include <string>
#include <unordered_map>

class ButtonSystem {
public:
    static ButtonSystem& getInstance() {
        static ButtonSystem instance;
        return instance;
    }
    
    //for gamepad
    ButtonAction getPendingAction() const { return m_pendingAction; }
    void setPendingAction(ButtonAction s) { m_pendingAction = s; }
    const std::vector<GameObject*>& getButtons() const { return m_buttons; }

    void init(GameObjectManager& manager);
    void refresh();
    void update();

private:
    void clearButtons();
    //void dispatchAction(ButtonAction action, const std::string& scenePath);
    bool isPointInButton(const Vector2D& point, GameObject* button) const;
    //void updateVolumeDisplay() const;
    void handleAction(const Button& btn);

    ButtonSystem() = default;
    ~ButtonSystem() = default;
    ButtonSystem(const ButtonSystem&) = delete;
    ButtonSystem& operator=(const ButtonSystem&) = delete;

    GameObjectManager* g_manager = nullptr;
    std::vector<GameObject*> m_buttons;
    ButtonAction m_pendingAction = ButtonAction::NONE;
    std::string  m_pendingScenePath = "";

    float m_previousVolume = 1.0f;

    //bool m_pendingSceneChange = false;

    int m_selectedButtonIdx = -1;
    size_t m_lastButtonCount = 0;
};
