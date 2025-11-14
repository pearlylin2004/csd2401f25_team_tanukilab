/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 90%
            Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
            for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Editor/editorManager.h"

DebugWindow::DebugWindow(DebugMode Dmode) : mode(Dmode){}

void DebugWindow::render() {
    const auto& messages = DebugLog::getMessages();

    std::string title = (mode == DebugMode::Editor) ? "Debug - Editor" : "Debug - Playing";
    ImGui::Begin(title.c_str());

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::BeginChild("DebugScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PopStyleVar();

    for (const auto& msg : messages) {
        if (msg.mode != mode) continue;
        if (mode == DebugMode::Editor) ImGui::TextWrapped("%s", msg.text.c_str());
        else ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.f, 1.f), "%s", msg.text.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}