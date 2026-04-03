/* Start Header ************************************************************************/
/*!
\file       ButtonSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       March, 27th, 2025
\brief      Declares the ButtonSystem class, which manages all interactive UI buttons
            in the scene. Handles mouse hover, click detection, audio feedback,
            and gamepad navigation for Button-component objects.

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

/**
 * @class ButtonSystem
 * @brief Singleton system that manages all Button-component objects in the active scene.
 *
 * Responsibilities:
 * - Detecting mouse hover and click on buttons using world-space AABB checks.
 * - Playing hover/click audio feedback via AudioComponent channels.
 * - Dispatching ButtonActions (load scene, pause, restart, exit, volume, etc.)
 * - Supporting gamepad D-pad navigation and A-button confirmation.
 * - Applying a hover darkening effect to button render components.
 *
 * Call refresh() after any scene load to rebuild the internal button list.
 * Call update() once per frame during gameplay.
 */
class ButtonSystem {
public:
    /** @brief Returns the singleton instance. */
    static ButtonSystem& getInstance() {
        static ButtonSystem instance;
        return instance;
    }
    
    /** @brief Returns the currently pending gamepad button action. */
    ButtonAction getPendingAction() const { return m_pendingAction; }

    /** @brief Sets the pending gamepad button action. */
    void setPendingAction(ButtonAction s) { m_pendingAction = s; }

    /** @brief Returns the list of registered button GameObjects in the current scene. */
    const std::vector<GameObject*>& getButtons() const { return m_buttons; }

    /** @brief Initializes the system with a reference to the active GameObjectManager. */
    void init(GameObjectManager& manager);

    /**
     * @brief Rebuilds the internal button list from the current scene.
     * Must be called after every scene load or scene change to ensure
     * m_buttons reflects the current set of Button-component objects.
     */
    void refresh();

    /**
     * @brief Processes mouse and gamepad input for all registered buttons.
     * Handles hover state, click detection, audio feedback, and action dispatch.
     * Skipped during editor mode or when the editor UI is shown.
     */
    void update();

private:
    /** @brief Clears the internal button list. */
    void clearButtons();

    /**
     * @brief Returns true if the given world-space point is inside the button's AABB.
     * Uses the button object's Transform scale as the collider bounds.
     */
    bool isPointInButton(const Vector2D& point, GameObject* button) const;

    /**
     * @brief Dispatches the action associated with a clicked button.
     * Routes to SceneManager, AudioHandler, or GLFW depending on the ButtonAction type.
     */
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
