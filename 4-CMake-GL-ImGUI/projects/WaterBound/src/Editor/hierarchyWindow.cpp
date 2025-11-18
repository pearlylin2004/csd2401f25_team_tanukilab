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

HierarchyWindow::HierarchyWindow(Editor::ObjSelectionState& Ostate) : m_objSelectionState(Ostate) {}

void HierarchyWindow::render(GameObjectManager& manager) {
    ImGui::Begin("Hierarchy");

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    // by default just set to no children
    ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;

    for (size_t i = 0; i < gameObjects.size(); i++) {
        // if no children (in the future)
    //    baseFlags |= ImGuiTreeNodeFlags_Leaf;

        // highlight selected row
        if (i == m_objSelectionState.selectedIndex) {
            baseFlags |= ImGuiTreeNodeFlags_Selected;
        }
        else {
            baseFlags &= ~ImGuiTreeNodeFlags_Selected;
        }

        // draw tree node for each object
        if (ImGui::TreeNodeEx(gameObjects[i]->getObjectName().c_str(), baseFlags)) {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                m_objSelectionState.selectedIndex = static_cast<int>(i);
                m_objSelectionState.selectedObject = gameObjects[i];
            }

            /* ------- right click context menu ------- */
            showContextMenu(manager, gameObjects[i], i);

            ImGui::TreePop();

        }
    }

    if (gameObjects.empty()) {
        m_objSelectionState.selectedObject = nullptr;
        m_objSelectionState.draggedObject = nullptr;
        m_objSelectionState.selectedIndex = -1;
        ImGui::Text("No GameObjects available");
    }

    /* ----------- Receive Dropped Prefab from Asset Browser ----------- */
    // make entire hierarchy viewport a invisible button that can receive asset
    //ImGuiID targetID = ImGui::GetID("HierarchyWindowDropTarget");
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::InvisibleButton("##drop_target", ImGui::GetContentRegionAvail());

    // accept the prefab drop
    handlePrefabDragDrop(manager);
    /* ---------- END --------- */

    ImGui::End();

    /* ---------- Popup Modal Call --------- */
    objDeletePopup(manager);
    createPrefabPopup(manager);
    /* ---------- END --------- */
}

void HierarchyWindow::handlePrefabDragDrop(GameObjectManager& manager) {
    // receive prefab and make it into a game obj referecing the prefab
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB")) {
            const char* prefabPath = static_cast<const char*>(payload->Data);

            std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(prefabPath);

            std::string objName = PrefabManager::Instance().getPrefabName(prefabID);
            GameObject* prefabObj = manager.createGameObject(objName + "(" + std::to_string(manager.getGameObjectCount()) + ")");
            prefabObj->getObjectPrefabID() = prefabID;
            if (prefabObj) {
                PrefabManager::Instance().instantiate(prefabObj, manager);
                Render* render = prefabObj->getComponent<Render>();
                if (render) render->texChanged = true;

                DebugLog::addMessage("Added game obj from prefab\n");
            }
            else {
                DebugLog::addMessage("Failed to receive\n");
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void HierarchyWindow::showContextMenu(GameObjectManager& manager, GameObject* obj, int objIndex) {
    /* ------- right click context menu ------- */
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Add Empty")) {
            AddObjWindow::createEmpty(manager);
        }

        if (ImGui::MenuItem("Add Object")) {
            AddObjWindow::createRec(manager);
        }

        if (ImGui::MenuItem("Duplicate")) {
            AddObjWindow::dupObj(manager, obj);
        }

        if (ImGui::MenuItem("Delete Object")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showDeletePopup = true;
        }

        if (ImGui::MenuItem("Save as Prefab")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showCreatePrefabPopup = true;
        }

        ImGui::EndPopup();
    }
}

void HierarchyWindow::objDeletePopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_objSelectionState.showDeletePopup) {
        ImGui::OpenPopup("Confirm Delete Object");
        m_objSelectionState.showDeletePopup = false;
    }

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Delete Object", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");

        if (ImGui::Button("Delete")) {
            std::string name = m_objSelectionState.selectedObject->getObjectName();

            manager.deleteGameObject(m_objSelectionState.selectedObject);

            m_objSelectionState.selectedObject = nullptr;
            m_objSelectionState.draggedObject = nullptr;
            m_objSelectionState.selectedIndex = -1;

            DebugLog::addMessage("Object " + name + " deleted\n");

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void HierarchyWindow::createPrefabPopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_objSelectionState.showCreatePrefabPopup) {
        ImGui::OpenPopup("Confirm Create Prefab");
        m_objSelectionState.showCreatePrefabPopup = false;
    }

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Create Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Do you want to pack this object into the new prefab?");

        if (ImGui::Button("Create and Pack")) {

            if (PrefabManager::Instance().createPrefabFromGameObj(m_objSelectionState.selectedObject, "", true))
                DebugLog::addMessage("Prefab created.\n");
            else DebugLog::addMessage("Prefab not created.\n");

            // attach the prefab id to the obj in json
            manager.saveScene(JsonIO::sourceScenePath(Editor::sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(Editor::sceneState.currentSceneName);

            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Create but Do Not Pack")) {

            if (PrefabManager::Instance().createPrefabFromGameObj(m_objSelectionState.selectedObject))
                DebugLog::addMessage("Prefab created.\n");
            else DebugLog::addMessage("Prefab not created.\n");

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}