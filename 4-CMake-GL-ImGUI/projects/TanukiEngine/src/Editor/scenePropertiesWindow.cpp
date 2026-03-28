/* Start Header ************************************************************************/
/*!
\file       scenePropertiesWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       March, 26th, 2026
\brief      Scene window in editor to assign on scene lua script to current scene

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

//#ifdef _DEBUG
#include "Editor/editorManager.h"

void ScenePropertiesWindow::render() {
    ImGui::Begin("Scene Properties");

    ImGui::Text("Scene: %s", m_sceneState.currentSceneName.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("On Load Script");

    std::string current = m_manager.getOnLoadScript();
    static char buf[128] = {};
    strncpy_s(buf, current.c_str(), sizeof(buf) - 1);

    if (ImGui::InputText("##OnLoadScript", buf, sizeof(buf),
        ImGuiInputTextFlags_EnterReturnsTrue)) {
        m_manager.setOnLoadScript(buf);
        DebugLog::addMessage("Set on-load script: " + std::string(buf) + "\n");
    }

    ImGui::TextDisabled("ENTER to apply, or drag .lua from asset browser");

    // drag drop .lua from asset browser
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
            std::filesystem::path dropped = (const char*)payload->Data;
            if (dropped.extension() == ".lua") {
                std::string normalized = dropped.string();
                size_t pos = normalized.find("assets");
                if (pos != std::string::npos)
                    normalized = normalized.substr(pos);
                m_manager.setOnLoadScript(normalized);

                DebugLog::addMessage("Set on-load script: " + normalized + "\n");
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (!m_manager.getOnLoadScript().empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            m_manager.setOnLoadScript("");
            DebugLog::addMessage("Cleared on-load script\n");
        }
    }

    ImGui::End();
}
//#endif