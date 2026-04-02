/* Start Header ************************************************************************/
/*!
\file        toastSystem.cpp
\author      Tan Chee Qing, cheeqing.tan, 2401486, 100%
\par         cheeqing.tan@digipen.edu
\date        Mar, 28, 2026
\brief       Toast system that render notification when called and auto
                -dismiss after a duration. Call push() to show a toast, and update() and render() inside the main loop.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "toastSystem.h"

ToastSystem& ToastSystem::getInstance() {
    static ToastSystem instance;
    return instance;
}

void ToastSystem::push(const std::string& msg, float duration) {
    m_toasts.push_back({ msg, 0.f, duration });
}

void ToastSystem::update(float deltaTime) {
    for (auto& t : m_toasts) t.timer += deltaTime;
    m_toasts.erase(
        std::remove_if(m_toasts.begin(), m_toasts.end(),
            [](const Toast& t) { return t.timer >= t.duration; }),
        m_toasts.end());
}

void ToastSystem::render() {
    if (m_toasts.empty()) return;

    ImGuiIO& io = ImGui::GetIO();
    float padding = 16.f;
    float startY = io.DisplaySize.y - padding;

    for (int i = (int)m_toasts.size() - 1; i >= 0; --i) {
        const Toast& t = m_toasts[i];

        // fade out in last second
        float alpha = 1.f;
        float remaining = t.duration - t.timer;
        if (remaining < 1.f) alpha = remaining;

        ImGui::SetNextWindowBgAlpha(0.85f * alpha);
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x - padding, startY),
            ImGuiCond_Always, ImVec2(1.f, 1.f));
        ImGui::SetNextWindowSize(ImVec2(280, 0));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        char winId[32];
        snprintf(winId, sizeof(winId), "##toast_%d", i);

        ImGui::PushStyleColor(ImGuiCol_Text,
            ImVec4(1.f, 1.f, 1.f, alpha));

        if (ImGui::Begin(winId, nullptr, flags)) {
            ImGui::TextWrapped("%s", t.message.c_str());
        }
        ImGui::End();
        ImGui::PopStyleColor();

        // stack toasts upward
        ImVec2 size = ImGui::GetItemRectSize();
        startY -= (size.y + padding + 10.f);
    }
}