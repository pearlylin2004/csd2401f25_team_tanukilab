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
    static ToastSystem& getInstance();

    void push(const std::string& msg, float duration = 5.f);

    void update(float deltaTime);

    // Call this inside an active ImGui frame
    void render();

private:
    std::vector<Toast> m_toasts;
};