/* Start Header ************************************************************************/
/*!
\file       hierarchyWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of Hierarchy in editor. Also
            include the right click context menu for game object manipulation (dup/delete)
            and creation of new object/prefab.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"
#include "Systems.h"

HierarchyWindow::HierarchyWindow(Editor::ObjSelectionState& Ostate) : m_objSelectionState(Ostate) {}

void HierarchyWindow::render(GameObjectManager& manager) {
    ImGui::Begin("Hierarchy");

    std::vector<GameObject*> gameObjects = CachedGameObjects::Instance().getCachedObjects();

    // by default just set to no children
    //ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;

    // Build parent → children map
    std::unordered_map<std::string, std::vector<GameObject*>> childrenOf;
    std::map<int, std::vector<GameObject*>> layerRoots; // layerID -> root objects

    for (GameObject* obj : gameObjects) {
        Transform* t = obj->getComponent<Transform>();
        if (!t || t->parentName.empty()) {
            layerRoots[obj->getLayer()].push_back(obj);
        }
        else {
            childrenOf[t->parentName].push_back(obj);
        }
    }

    // Sort alphabetically within each layer
    for (auto& [layerID, objs] : layerRoots) {
        std::sort(objs.begin(), objs.end(), [](const GameObject* a, const GameObject* b) { return a->getObjectName() < b->getObjectName();} );
    }

    // Recursive lambda to render an object and its children
    std::function<void(GameObject*)> renderNode = [&](GameObject* obj) {
        const std::string& name = obj->getObjectName();
        bool hasChildren = childrenOf.count(name) && !childrenOf[name].empty();
        bool isSelected = (m_objSelectionState.selectedObject == obj);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
        if (isSelected)   flags |= ImGuiTreeNodeFlags_Selected;

        bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

        // Selection
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            GameObject* prevSelected = m_objSelectionState.selectedObject;
            if (prevSelected && prevSelected != obj && prevSelected->hasComponent<TileMap>()) {
                TileMapSystem::filename.clear();
                TileMapSystem::eraseMode = false;
            }
            m_objSelectionState.selectedObject = obj;
            // Update index to match
            auto it = std::find(gameObjects.begin(), gameObjects.end(), obj);
            m_objSelectionState.selectedIndex = (it != gameObjects.end())
                ? static_cast<int>(std::distance(gameObjects.begin(), it)) : -1;
        }

        // Drag source — emit object name so inspector can receive it for parenting
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("HIERARCHY_OBJ", name.c_str(), name.size() + 1);
            ImGui::Text("Parent to: %s", name.c_str());
            ImGui::EndDragDropSource();
        }

        // Drop target — drag one object onto another to reparent
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJ")) {
                const char* childName = (const char*)payload->Data;
                if (childName && std::string(childName) != name) {
                    GameObject* child = manager.getGameObject(childName);
                    if (child && child->hasComponent<Transform>() && obj->hasComponent<Transform>()) {
                        Transform* ct = child->getComponent<Transform>();
                        Transform* pt = obj->getComponent<Transform>();
                        float dx = ct->x - pt->x;
                        float dy = ct->y - pt->y;
                        float rad = glm::radians(-pt->rotation);
                        float cosA = cosf(rad), sinA = sinf(rad);

                        // Capture before state
                        std::string oldParent = ct->parentName;
                        Vector2D    oldOffset = ct->localOffset;
                        float       oldLocalRot = ct->localRotation;

                        ct->localOffset.x = dx * cosA - dy * sinA;
                        ct->localOffset.y = dx * sinA + dy * cosA;
                        ct->localRotation = ct->rotation - pt->rotation;
                        ct->parentName = name;

                        // Record for undo
                        auto cmd = std::make_unique<ReparentCmd>(manager, childName,
                            oldParent, oldOffset, oldLocalRot,
                            ct->parentName, ct->localOffset, ct->localRotation);
                        UndoRedoManager::Instance().executeCmd(std::move(cmd));

                        DebugLog::addMessage(childName + std::string(" parented to ") + name);
                        CachedGameObjects::Instance().refresh();
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        showContextMenu(manager, obj,
            static_cast<int>(std::distance(gameObjects.begin(),
                std::find(gameObjects.begin(), gameObjects.end(), obj))));

        if (nodeOpen) {
            // Recurse into children
            if (hasChildren) {
                for (GameObject* child : childrenOf[name]) {
                    renderNode(child);
                }
            }
            ImGui::TreePop();
        }
    };

    // Render root objects grouped by layer under collapsible headers
    LayerManager& layerMgr = manager.getLayerManager();
    for (auto& [layerID, objs] : layerRoots) {
        Layer* layer = layerMgr.getLayer(layerID);
        std::string headerLabel = layer
            ? layer->getLayerName() + "  (" + std::to_string(objs.size()) + ")"
            : "Layer " + std::to_string(layerID);

        bool layerEnabled = layer ? layer->isEnabled() : true;
        if (!layerEnabled)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));

        bool open = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

        if (!layerEnabled) ImGui::PopStyleColor();

        if (open) {
            for (GameObject* root : objs) {
                renderNode(root);
            }
        }
    }

    if (gameObjects.empty()) {
        m_objSelectionState.selectedObject = nullptr;
        m_objSelectionState.draggedObject = nullptr;
        m_objSelectionState.selectedIndex = -1;

        ImGui::Text("No GameObjects available");
        if (ImGui::Button("Create Empty")) {
            AddObjWindow::createEmpty(manager);
        }
    }

    /* ----------- Receive Dropped Prefab from Asset Browser ----------- */
    // make entire hierarchy viewport a invisible button that can receive asset
    //ImGuiID targetID = ImGui::GetID("HierarchyWindowDropTarget");
    // Use window-rect drop target instead of InvisibleButton
    // This doesn't interfere with tree node drop targets
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (ImGui::BeginDragDropTargetCustom(window->ContentRegionRect, window->ID)) {
        handlePrefabDragDrop(manager);
        ImGui::EndDragDropTarget();
    }

    // accept the prefab drop
    //handlePrefabDragDrop(manager);

    /* ---------- END --------- */

    ImGui::End();

    /* ---------- Popup Modal Call --------- */
    objDeletePopup(manager);
    createPrefabPopup(manager);
    /* ---------- END --------- */
}

void HierarchyWindow::handlePrefabDragDrop(GameObjectManager& manager) {
    // receive prefab and make it into a game obj referecing the prefab
    // BeginDragDropTarget is handled by the caller
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
        const char* prefabPath = static_cast<const char*>(payload->Data);
        std::filesystem::path path = prefabPath;
        //std::string folder = path.parent_path().filename().string();

        // check if "Prefab" appears anywhere in the path (handles subfolders)
        bool isInPrefabFolder = false;
        for (const std::filesystem::path& part : path) {
            if (part.string() == "Prefab") {
                isInPrefabFolder = true;
                break;
            }
        }

        // only accept if its in prefab folder
        if (isInPrefabFolder && path.extension() == ".json") {
            std::string fullPath = path.string();
            std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

            std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(AssetBrowser::toRegistryPath(path.string()));
            if (prefabID.empty()) {
                DebugLog::addMessage("Cannot find prefab ID for: " + path.string());
                return;
            }

            std::string objName = PrefabManager::Instance().getPrefabName(prefabID) + "(" + std::to_string(manager.getGameObjectCount()) + ")";
            GameObject* prefabObj = manager.createGameObject(objName);
            prefabObj->getObjectPrefabID() = prefabID;

            if (prefabObj) {
                PrefabManager::Instance().instantiate(prefabObj, manager);
                Render* render = prefabObj->getComponent<Render>();
                if (render) render->texChanged = true;

                auto cmd = std::make_unique<CreateObjectCmd>(manager, objName);
                UndoRedoManager::Instance().executeCmd(std::move(cmd));

                // refresh cached object vector
                CachedGameObjects::Instance().refresh();

                DebugLog::addMessage("Added game obj from prefab\n");
            }
            else {
                Editor::popupState.prefabFileTypePopup = true;
                Editor::popupState.filename = path.filename().string();
                DebugLog::addMessage("Failed to receive\n");
            }
        }
        else {
            Editor::popupState.prefabFileTypePopup = true;
            Editor::popupState.filename = path.filename().string();

            DebugLog::addMessage("Wrong prefab file received: " + path.filename().string() + "\n");
        }
    }
    
}

void HierarchyWindow::showContextMenu(GameObjectManager& manager, GameObject* obj, int objIndex) {
    /* ------- right click context menu ------- */
    if (ImGui::BeginPopupContextItem()) {

        if (ImGui::MenuItem("Duplicate")) {
            AddObjWindow::dupObj(manager, obj);
        }

        if (ImGui::MenuItem("Delete Object")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showDeletePopup = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::MenuItem("Save as Prefab")) {
            m_objSelectionState.selectedIndex = objIndex;
            m_objSelectionState.selectedObject = obj;
            m_objSelectionState.showCreatePrefabPopup = true;
        }

        // if selected object is ref a prefab
        if (!obj->getObjectPrefabID().empty()) {
            if (ImGui::MenuItem("Edit Prefab")) {
                std::unique_ptr<GameObject> tempPrefab = PrefabManager::Instance().createTempPrefabObj(obj->getObjectPrefabID());
                if (tempPrefab) {
                    m_objSelectionState.selectedObject = nullptr;
                    m_objSelectionState.selectedIndex = -1;
                    m_objSelectionState.selectedPrefab = std::move(tempPrefab);

                    DebugLog::addMessage("Open Prefab " + m_objSelectionState.selectedPrefab->getObjectName() + ".");
                }
                else {
                    DebugLog::addMessage("Failed to open prefab " + obj->getObjectPrefabID() + " object for editing.\n");
                }
            }

            if (ImGui::MenuItem("Revert to Prefab")) {
                /*obj = PrefabManager::Instance().revertToPrefab(obj, manager);*/
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RevertToPrefabCmd>(manager, obj));

                DebugLog::addMessage("Reverted " + obj->getObjectName() + " to its prefab" + obj->getObjectPrefabID() + ".");
            }

            if (ImGui::MenuItem("Unpack Prefab")) {
                //obj->getObjectPrefabID().clear();
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<UnpackPrefabCmd>(manager, obj));

                DebugLog::addMessage("Unpacked prefab: " + obj->getObjectName() + " is now a regular object.");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::MenuItem("Create Empty")) {
            AddObjWindow::createEmpty(manager);
        }

        if (ImGui::MenuItem("Create Object")) {
            AddObjWindow::createRec(manager);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Add inside showContextMenu, after the existing Duplicate/Delete items:
        Transform* t = obj->getComponent<Transform>();
        if (t && !t->parentName.empty()) {
            if (ImGui::MenuItem("Unparent")) {
                t->parentName.clear();
                t->localOffset = { 0.f, 0.f };
                t->localRotation = 0.f;
                CachedGameObjects::Instance().refresh();
                DebugLog::addMessage(obj->getObjectName() + " unparented");
            }
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


    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Delete Object", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this object?");

        if (ImGui::Button("Delete") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::string name = m_objSelectionState.selectedObject->getObjectName();

            if (Editor::visualLogicState.targetObj == m_objSelectionState.selectedObject) {
                VisualLogicEditor::Instance().clear();
            }

            auto cmd = std::make_unique<DeleteObjCmd>(manager, m_objSelectionState.selectedObject);
            UndoRedoManager::Instance().executeCmd(std::move(cmd));

            // refresh cached object vector
            CachedGameObjects::Instance().refresh();

            m_objSelectionState.selectedObject = nullptr;
            m_objSelectionState.draggedObject = nullptr;
            m_objSelectionState.selectedIndex = -1;

            DebugLog::addMessage("Object " + name + " deleted");

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

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    /*---------------- Cannot delete current loaded scene ----------------*/
    // delete confirm popup
    if (ImGui::BeginPopupModal("Confirm Create Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Do you want to pack this object into the new prefab?");

        if (m_objSelectionState.selectedObject) {
            bool hasTileMap = m_objSelectionState.selectedObject->hasComponent<TileMap>();
            bool hasCamera = m_objSelectionState.selectedObject->hasComponent<camera>();
            if (hasTileMap || hasCamera) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "Warning:");
                if (hasTileMap)
                    ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "  - TileMap is a special component and will not be saved to the prefab.");
                if (hasCamera)
                    ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "  - Camera is a special component and will not be saved to the prefab.");
                ImGui::Spacing();
            }

            ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "  - Layer is a special component and will not be saved to the prefab.");
        }

        if (ImGui::Button("Create and Pack")) {

            if (PrefabManager::Instance().createPrefabFromGameObj(m_objSelectionState.selectedObject, "", true)) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<PackPrefabCmd>(manager, m_objSelectionState.selectedObject));

                DebugLog::addMessage("Prefab created.\n");
            }
            else DebugLog::addMessage("Prefab not created.\n");

            // attach the prefab id to the obj in json (auto save)
            /*manager.saveScene(JsonIO::sourceScenePath(Editor::sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(Editor::sceneState.currentSceneName);*/

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
#endif