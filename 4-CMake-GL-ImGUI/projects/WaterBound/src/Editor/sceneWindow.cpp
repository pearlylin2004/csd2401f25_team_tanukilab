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

SceneWindow::SceneWindow(AssetBrowser& aBrowser, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate, Editor::SceneWindowState& Sstate, Editor::SceneState& sceneState) :
    m_assetBrowser(aBrowser), 
    m_objSelectionState(Ostate),
    m_menuBarState(Mstate),
    m_sceneWindowState(Sstate),
    m_sceneState(sceneState) {}

void SceneWindow::render(GLuint texture, float aspectRatio, GameObjectManager& manager) {
    ImGui::Begin("Scene", nullptr);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 size = avail;

    // calculate how much to scale based on aspect ratio
    float availAspect = avail.x / avail.y;
    if (availAspect > aspectRatio) size.x = avail.y * aspectRatio;
    else size.y = avail.x / aspectRatio;

    // center in the available space
    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 offset = { (avail.x - size.x) * 0.5f, (avail.y - size.y) * 0.5f };
    if (offset.x < 0) offset.x = 0;
    if (offset.y < 0) offset.y = 0;
    ImGui::SetCursorPos({ cursor.x + offset.x, cursor.y + offset.y });


    // draw the scene in a child window to prevent moving/resizing this child viewport
    ImGui::BeginChild("SceneViewport", size, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Image((void*)(intptr_t)texture, size, ImVec2(0, 1), ImVec2(1, 0));

    if (ImGui::IsWindowHovered())
    {
        // Pan with middle mouse button
        if (InputHandler::isMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE))
        {
            renderer::editorCam.pan(InputHandler::getMouseDeltaWorldInViewport(m_sceneWindowState.sceneSize));
        }

        if (InputHandler::getMouseScroll() != 0.f)
        {
            renderer::editorCam.zoomInOut(InputHandler::getMouseScroll());
        }

        if (InputHandler::isMouseLeftClicked()) {
            objPicking(manager);
        }

        if (InputHandler::isMouseDragging()) {
            objDragging();
        }

    }

    ImGui::EndChild();


    m_sceneWindowState.scenePos = ImGui::GetItemRectMin();
    m_sceneWindowState.sceneSize = ImGui::GetItemRectSize();

    ImGui::End();

    /* ---------- Popup Modal Call --------- */
    newScenePopup(manager);
    duplicateScenePopup(manager);
    loadScenePopup(manager);
    /* ---------- END --------- */
}

void SceneWindow::newScene(GameObjectManager& manager) {
    //std::string folder = std::string(SOURCE_SCENE_DIR_R);
    std::string base = "untitled_scene";
    std::string extension = ".json";
    std::string newSceneName;

    int copy = 1;
    bool nameExists;
    do {
        nameExists = false;
        newSceneName = base + "(" + std::to_string(copy) + ")" + extension;

        for (const Editor::Asset& asset : m_assetBrowser.getAssets()) {
            if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
                nameExists = true;
                break;
            }
        }

        copy++;
    } while (nameExists);

    m_sceneState.currentSceneName = newSceneName;
    manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName), true);
    JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);
    EditorManager::assetChanged();

    m_objSelectionState.selectedObject = nullptr;
    m_objSelectionState.draggedObject = nullptr;
    m_objSelectionState.selectedPrefab = nullptr;
    m_objSelectionState.selectedIndex = -1;

    DebugLog::addMessage("New scene " + m_sceneState.currentSceneName + " created.\n");
}

// dup current scene and open
void SceneWindow::duplicateScene(GameObjectManager& manager) {
    if (!m_sceneState.currentSceneName.empty()) {
        std::filesystem::path originalName(m_sceneState.currentSceneName);
        std::string folder = originalName.parent_path().string();
        std::string filename = originalName.stem().string();
        std::string extension = originalName.extension().string();

        // check for existing names and find a new unique name
        std::string newSceneName;
        int copy = 1;
        bool nameExists;
        do {
            nameExists = false;
            newSceneName = folder + "/" + filename + "_copy(" + std::to_string(copy) + ")" + extension;

            for (const Editor::Asset& asset : m_assetBrowser.getAssets()) {
                if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
                    nameExists = true;
                    break;
                }
            }

            copy++;
        } while (nameExists);

        //manager.saveToFile(newSceneName);
        m_sceneState.currentSceneName = newSceneName;
        manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
        JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

        // if current scene got /, remove it (to compare and highlight this asset in browser)
        if (m_sceneState.currentSceneName[0] == '/') {
            m_sceneState.currentSceneName.erase(0, 1);
        }
        EditorManager::assetChanged();

        m_objSelectionState.selectedObject = nullptr;
        m_objSelectionState.draggedObject = nullptr;
        m_objSelectionState.selectedPrefab = nullptr;
        m_objSelectionState.selectedIndex = -1;
        DebugLog::addMessage("Scene duplicated to: " + newSceneName + "\n");
    }
    else {
        DebugLog::addMessage("No current scene to duplicate.\n");
    }
}

// load existing scene
void SceneWindow::loadScene(GameObjectManager& manager) {
    //manager.loadFromFile(m_MenuBarState.sceneToLoad);
    manager.loadScene(JsonIO::runtimeScenePath(m_menuBarState.sceneToLoad));

    m_sceneState.currentSceneName = std::move(m_menuBarState.sceneToLoad);

    m_objSelectionState.selectedObject = nullptr;
    m_objSelectionState.draggedObject = nullptr;
    m_objSelectionState.selectedPrefab = nullptr;
    m_objSelectionState.selectedIndex = -1;

    DebugLog::addMessage("Loaded scene: " + m_sceneState.currentSceneName + "\n");
}

void SceneWindow::newScenePopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_menuBarState.showNewScenePopup) {
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm New Scene"); // only load scene when not playing simulation
        else DebugLog::addMessage("Playing simulation, cannot load another scene.\n");
        m_menuBarState.showNewScenePopup = false;
    }

    if (ImGui::BeginPopupModal("Confirm New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create a new scene? Unsaved changes will be lost.");

        // Save and open new scene
        if (ImGui::Button("Save and Create New")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            // create new scene
            newScene(manager);

            // load new scene
            m_menuBarState.sceneToLoad = m_sceneState.currentSceneName;
            loadScene(manager);

            ImGui::CloseCurrentPopup();
        }

        // Discard changes and open new scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {

            // create new scene
            newScene(manager);

            // load new scene
            m_menuBarState.sceneToLoad = m_sceneState.currentSceneName;
            loadScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneWindow::duplicateScenePopup(GameObjectManager& manager) {
    if (m_menuBarState.showDuplicatePopup) {
        m_menuBarState.showDuplicatePopup = false;
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm Duplicate Scene"); // only duplicate scene when not playing simulation
        else DebugLog::addMessage("Playing simulation, cannot load scene.\n");
    }

    if (ImGui::BeginPopupModal("Confirm Duplicate Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Duplicate and open new a scene? Unsaved changes will be lost.");

        // Save and open newly duplicated scene
        if (ImGui::Button("Save and Duplicate")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Discard changes and open newly duplicated scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {

            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneWindow::loadScenePopup(GameObjectManager& manager) {
    if (m_menuBarState.showLoadScenePopup) {
        m_menuBarState.showLoadScenePopup = false;
        if (EditorManager::isEditingMode()) ImGui::OpenPopup("Confirm Load Scene"); // only load scene when not playing simulation
        else DebugLog::addMessage("Playing simulation, cannot load scene.\n");
    }

    if (ImGui::BeginPopupModal("Confirm Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        // show different title if reloading current scene
        std::string title = (m_menuBarState.sceneToLoad == m_sceneState.currentSceneName) ? "Reload current scene? Unsaved changes will be lost." : "Load another scene? Unsaved changes will be lost";

        ImGui::Text(title.c_str());

        // Save and load another scene
        if (ImGui::Button("Save and Load")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(JsonIO::sourceScenePath(m_sceneState.currentSceneName));
            JsonIO::syncSceneToRuntime(m_sceneState.currentSceneName);

            loadScene(manager);
            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            for (GameObject* object : gameObjects) {
                Render* r = object->getComponent<Render>();
                if (r) {
                    r->texChanged = true;
                }
            }
            ImGui::CloseCurrentPopup();
        }

        // Discard changes and load selected scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {
            loadScene(manager);

            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            for (GameObject* object : gameObjects) {
                Render* r = object->getComponent<Render>();
                if (r) {
                    r->texChanged = true;
                }
            }
            ImGui::CloseCurrentPopup();
        }

        // Cancel loading and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/*
Future implementation (jic i forget)
- Add snapping to grid or axis (e.g. hold Shift for 1-unit steps)

- Highlight selected object while dragging

- Undo/redo
*/
void SceneWindow::objPicking(GameObjectManager& manager) {
    Vector2D mousePos = InputHandler::getMousePositionInImGuiViewport(m_sceneWindowState.scenePos, m_sceneWindowState.sceneSize);

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    GameObject* selected = nullptr;
    GameObject* topmost = nullptr;
    float highestZ = -INFINITY;

    for (auto* obj : gameObjects) {
        // dont let user click and select platform
        if (obj->getObjectName() == "platform") continue;

        Transform* transform = obj->getComponent<Transform>();
        if (!transform) continue;

        Collision::AABB aabb = Collision::getObjectAABB(transform);
        Collision::Circle mousePoint(mousePos, 0.2f);

        // check if mouse point intersects with the game obj
        if (Collision::CollisionIntersection_CircleAABB_Static(mousePoint, aabb)) {
            if (transform->z > highestZ) {
                highestZ = transform->z;
                topmost = obj;
            }
        }
    }

    selected = topmost;
    // if something is selected, update selection state
    if (selected) {
        m_objSelectionState.selectedPrefab = nullptr;
        m_objSelectionState.selectedObject = selected;
        m_objSelectionState.draggedObject = selected;

        auto iterator = std::find(gameObjects.begin(), gameObjects.end(), selected);
        if (iterator != gameObjects.end()) {
            m_objSelectionState.selectedIndex = static_cast<int>(std::distance(gameObjects.begin(), iterator));
        }

        DebugLog::addMessage(selected->getObjectName() + " selected");
    }
    else {
        // if mouse click somewhere else, deselect current dragged object
        m_objSelectionState.draggedObject = nullptr;
    }
}

void SceneWindow::objDragging() const {
    // early return if no obj is selected
    GameObject* obj = m_objSelectionState.draggedObject;
    if (!obj) return;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform) return;

    // change current object position based on mouse delta
    Vector2D mousePos = InputHandler::getMouseDeltaWorldInViewport(m_sceneWindowState.sceneSize);
    transform->x += mousePos.x;
    transform->y += mousePos.y;

    // if obj has physics, do not follow gravity while dragging
    Physics* physics = obj->getComponent<Physics>();
    if (!physics) return;

    physics->velY = 0.0f;
}