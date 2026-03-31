/* Start Header ************************************************************************/
/*!
\file        toastSystem.h
\author      Tan Chee Qing, cheeqing.tan, 2401486, 100%
\par         cheeqing.tan@digipen.edu
\date        Mar, 28, 2026
\brief       Toast system that render notification when called.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <imgui.h>

struct Toast {
    std::string message;
    float       timer = 0.f;
    float       duration = 5.f; // auto-dismiss after 5 sec
};

class ToastSystem {
public:
    // Access the singleton instance of the ToastSystem
    static ToastSystem& getInstance();

    // Add a new toast message to be displayed
    // Parameters:
    //   msg: message to display
    //   duration: optional custom display duration
    void push(const std::string& msg, float duration = 5.f);

    // Update all active toasts (advance timers and remove expired ones)
    // Parameter:
    //   deltaTime: time passed since last frame
    void update(float deltaTime);

    // Render all active toast messages using ImGui
    // Should be called within an active ImGui frame
    void render();

private:
    // List of active toast notifications
    std::vector<Toast> m_toasts;
};