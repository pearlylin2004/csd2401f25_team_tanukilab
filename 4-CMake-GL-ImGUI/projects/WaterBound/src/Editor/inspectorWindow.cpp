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

InspectorWindow::InspectorWindow(Editor::ObjSelectionState& Ostate) : m_objSelectionState(Ostate) {}

void InspectorWindow::render(GameObjectManager& manager) {
    ImGui::Begin("Inspector");

    GameObject* selected = nullptr;
    if (m_objSelectionState.selectedObject) selected = m_objSelectionState.selectedObject;
    else if (m_objSelectionState.selectedPrefab) {
        selected = m_objSelectionState.selectedPrefab.get();

        if (ImGui::Button("Save")) {
            if (PrefabManager::Instance().savePrefab(selected)) {
                /*manager.loadScene(JsonIO::runtimeScenePath(Editor::sceneState.currentSceneName));*/
                DebugLog::addMessage("Prefab saved\n");
            }
            else {
                DebugLog::addMessage("Prefab not saved\n");
            }

        }

        // create another temp prefab obj of the same prefab to revert
        if (ImGui::Button("Revert")) {
            m_objSelectionState.selectedPrefab = PrefabManager::Instance().createTempPrefabObj(selected->getObjectPrefabID());
            selected = m_objSelectionState.selectedPrefab.get();
            DebugLog::addMessage("Prefab " + selected->getObjectName() + " reverted to original.\n");
        }
    }

    if (selected) {

        /* ------------------- Edit Object Name ------------------- */
        if (selected == m_objSelectionState.selectedObject) {
            std::string& objName = selected->getObjectName();
            char buffer[128];
            strncpy(buffer, objName.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
                std::string newObjName = buffer;
                if (manager.renameGameObject(m_objSelectionState.selectedObject, newObjName)) {
                    DebugLog::addMessage("Object renamed to " + newObjName + "\n");
                }
                else {
                    DebugLog::addMessage("Failed to rename: name already exists\n");
                }
            }
            ImGui::SetItemDefaultFocus();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        else {
            ImGui::Text("Prefab Name: "); ImGui::SameLine();
            ImGui::Text(selected->getObjectName().c_str());
        }


        /* ------------------- Object Transformation ------------------- */
        if (Transform* transform = selected->getComponent<Transform>()) {
            renderTransform(transform);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Change Texture ------------------- */
        // drag and drop received from asset window
        if (Render* render = selected->getComponent<Render>()) {
            renderRender(render, selected);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

    }
    else {
        ImGui::Text("No object selected");
    }

    ImGui::End();
}

void InspectorWindow::renderTransform(Transform* transform) {
    if (ImGui::CollapsingHeader("Transform")) {

        ImGui::DragFloat("X", &transform->x, 0.1f);
        ImGui::DragFloat("Y", &transform->y, 0.1f);

        // set z to stay within the editor camera (-1.f <= z < 1.f)
        glm::mat4& proj = renderer::editorCam.proj;
        float m22 = proj[2][2];
        float m32 = proj[3][2];
        float near1 = (m32 - 1.f) / m22;
        float far1 = (m32 + 1.f) / m22;
        ImGui::DragFloat("Z", &transform->z, 0.1f, far1, near1);
    }

    /* ------------------- Object Rotation ------------------- */
    if (ImGui::CollapsingHeader("Rotation")) {
        ImGui::DragFloat("Angle", &transform->rotation);
    }

    /* ------------------- Object Scale ------------------- */
    if (ImGui::CollapsingHeader("Scale")) {
        ImGui::Checkbox("Aspect Ratio (X only)", &m_objSelectionState.aspectRatioLock);

        if (m_objSelectionState.aspectRatioLock && !m_objSelectionState.ratioSet) {
            m_objSelectionState.ratioY = transform->scaleY / transform->scaleX;
            m_objSelectionState.ratioZ = transform->scaleZ / transform->scaleX;
            m_objSelectionState.ratioSet = true;
        }

        if (!m_objSelectionState.aspectRatioLock) m_objSelectionState.ratioSet = false;

        // clamp scale to be non-negative
        ImGui::DragFloat("X1", &transform->scaleX, 0.1f, 0.0f, FLT_MAX);
        ImGui::DragFloat("Y1", &transform->scaleY, 0.1f, 0.0f, FLT_MAX);
        ImGui::DragFloat("Z1", &transform->scaleZ, 0.1f, 0.0f, FLT_MAX);

        if (m_objSelectionState.aspectRatioLock && transform->scaleX != 0.0f) {
            transform->scaleY = transform->scaleX * m_objSelectionState.ratioY;
            transform->scaleZ = transform->scaleX * m_objSelectionState.ratioZ;
        }
    }
}

void InspectorWindow::renderRender(Render* render, GameObject* selected) {
    if (ImGui::CollapsingHeader("Render")) {
        ImGui::Text("Texture");
        ImGui::SameLine();

        std::string texName = render->texFile.empty() ? "(Drop new texture here)" : render->texFile;
        ImGui::Button(texName.c_str());

        /* -------------- Drag and Drop From Asset Browser to Change Texture ------------- */
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                const char* droppedPath = (const char*)payload->Data;
                std::string path = droppedPath;

                const std::string debugPrefix = "Debug/";
                size_t pos = path.find(debugPrefix);
                if (pos != std::string::npos) path = path.substr(pos + debugPrefix.size());

                /* --- File extension validation --- */
                std::string ext;
                size_t dotPos = path.find_last_of('.');
                if (dotPos != std::string::npos) ext = path.substr(dotPos + 1);

                // Convert to lowercase for consistency
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                // only accept if dropped file is a png/jpg/jpeg
                if (ext == "png" || ext == "jpg" || ext == "jpeg") {
                    render->texFile = path;
                    render->hasTex = true;
                    render->texChanged = true;
                    DebugLog::addMessage("Changed texture of " + selected->getObjectName() + " to " + render->texFile + "\n");
                }
                else {
                    // invalid texture format -> don't change anything
                    DebugLog::addMessage("Invalid texture format : " + path + "\n");
                }

            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        if (render->hasTex) {
            ImGui::Spacing();
            if (ImGui::Button("Clear Texture")) {
                glDeleteTextures(1, &render->texHDL);
                render->texHDL = 0;
                render->hasTex = false;
                render->texFile.clear();
                render->texChanged = false;
            }
        }
    }
}