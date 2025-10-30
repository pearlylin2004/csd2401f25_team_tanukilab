/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
            for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ui.h"

bool UI::assetsChangedFlag = true;

UI::UI() {

    m_currentSceneName = "shaders/Scene/temp_serialize.txt";
    m_tempSceneName = "shaders/Scene/temp_scene";
    m_isPlaying = false;
}

void UI::update(GameObjectManager& manager) {
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_LEFT_CONTROL) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_RIGHT_CONTROL))
        manager.saveToFile(m_currentSceneName);

    editorMenuBar(manager);
    objPicking(manager);
    objDragging();

    drawPerformance();
    drawShapes(manager);
    drawDebug();

    objInspectWindow(manager);
    assetWindow(manager);
}

void UI::addDebugMessage(const std::string& msg) {
    m_debugMessages.push_back(msg);
}

// edit shape windows
//void UI::shapeControl(Transform& t1, Transform& t2) {
//    if (ImGui::Begin("Shape Control")) {
//        ImGui::Text("Transform Component");
//        static ShapeType currentShape = ShapeType::Leaves;
//
//        if (ImGui::Button("Leaves")) {
//            currentShape = ShapeType::Leaves;
//        }
//        ImGui::SameLine();
//        if (ImGui::Button("Bullet")) {
//            currentShape = ShapeType::Bullet;
//        }
//
//        //check which shape is selected
//        Transform* t = (currentShape == ShapeType::Leaves) ? &t1 : &t2;
//        ShapeDefaults& defaults = m_shapePresets[currentShape];
//
//        //reset
//        if (ImGui::Button("Reset All")) {
//            if (m_shapePresets.find(currentShape) != m_shapePresets.end()) {
//                t->x = defaults.posX;
//                t->y = defaults.posY;
//                t->rotation = defaults.rotation;
//                t->scaleX = defaults.scaleX;
//                t->scaleY = defaults.scaleY;
//                t->scaleZ = defaults.scaleZ;
//            }
//            else {
//                m_debugMessages.push_back("No presets found for the selected shape.");
//            }
//        }
//    }
//    ImGui::End();
//}

void UI::drawPerformance() {
    ImGui::Begin("Performance");
    //time frame:
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);
    ImGui::Separator();
    //average time

    ImGui::End();
}

void UI::drawShapes(GameObjectManager& manager) {
    {
        ImGui::Begin("Draw Shapes");

        ImGui::PushItemWidth(150.0f);

        // For drawing circle
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X", &circleInput.inputX, 0.1f);
        ImGui::SameLine();

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y", &circleInput.inputY, 0.1f);
        ImGui::SameLine();

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z", &circleInput.inputZ, 0.1f);
        ImGui::SameLine();

        if (ImGui::Button("Add Circle")) {
            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            GameObject* circleObj = manager.createGameObject("Circle_" + std::to_string(gameObjects.size()));

            Transform* transform = circleObj->addComponent<Transform>();
            Render* render = circleObj->addComponent<Render>();
            render->modelRef = renderer::setup_circle({ 1.0f, 0.f, 0.f }, 100);

            transform->x = circleInput.inputX;
            transform->y = circleInput.inputY;
            transform->z = circleInput.inputZ;

            //auto* coll = circleObj->addComponent<CollisionInfo>();
            addDebugMessage("Circle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
        }

        ImGui::Separator();

        // For drawing rectangle        
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X1", &RectangleInput.inputX, 0.1f);
        ImGui::SameLine();

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y1", &RectangleInput.inputY, 0.1f);
        ImGui::SameLine();

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z1", &RectangleInput.inputZ, 0.1f);
        ImGui::SameLine();

        if (ImGui::Button("Add Rectangle")) {
            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            GameObject* recobj = manager.createGameObject("Rectangle_" + std::to_string(gameObjects.size()));

            Transform* transform = recobj->addComponent<Transform>();
            Render* render = recobj->addComponent<Render>();
            render->modelRef = renderer::setup_square({ 1.0f, 0.f, 0.f });

            transform->x = RectangleInput.inputX;
            transform->y = RectangleInput.inputY;
            transform->z = RectangleInput.inputZ;

            //auto* col = recobj->addComponent<CollisionInfo>();
            addDebugMessage("Rectangle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
        }

        ImGui::End();
    }
}

void UI::drawDebug() {
    ImGui::Begin("Debug Info");

    for (const auto& msg : m_debugMessages) {
        ImGui::Text("%s", msg.c_str());
    }

    //auto-scroll if at the bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}

void UI::editorMenuBar(GameObjectManager& manager) {
    ImGui::BeginMainMenuBar();

    /* ------------------- File Menu ------------------- */
    if (ImGui::BeginMenu("File")) {

        // create new scene
        if (ImGui::MenuItem("New Scene (coming soon)")) {
            // implement new scene logic here 
        }

        // duplicate scene
        if (ImGui::MenuItem("Duplicate Scene")) {
            if (!m_currentSceneName.empty()) {
                std::string newSceneName = m_currentSceneName;

                if (newSceneName.find_last_of('.') != std::string::npos) {
                    newSceneName.insert(newSceneName.find_last_of('.'), "_copy");
                }
                else {
                    newSceneName += "_copy";
                }

                manager.saveToFile(newSceneName);
                m_currentSceneName = newSceneName;
                assetsChangedFlag = true;

                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;
                addDebugMessage("Scene duplicated to: " + newSceneName + "\n");
            }
            else {
                addDebugMessage("No current scene to duplicate.\n");
            }
        }

        // save current scene to file
        if (ImGui::MenuItem("Save Scene")) {
            manager.saveToFile(m_currentSceneName);
        }

        // load an existing scene
        if (ImGui::BeginMenu("Load Scene")) {

            // load through the assets to find scene files
            for (const Asset& asset : m_assets) {
                if (asset.folder == m_AssetBrowserState.currentFolder + "Scene") {
                    if (ImGui::MenuItem(asset.name.c_str())) {
                        std::string fullPath = asset.folder + "/" + asset.name;
                        manager.loadFromFile(fullPath);

                        m_currentSceneName = fullPath;

                        m_SelectionState.selectedObject = nullptr;
                        m_SelectionState.draggedObject = nullptr;
                        m_SelectionState.selectedIndex = -1;

                        addDebugMessage("Loaded scene: " + asset.name + "\n");
                    }
                }
            }
            ImGui::EndMenu();

        }

        ImGui::EndMenu();
    }

    /* ------------------- Simulation ------------------- */
    if (ImGui::BeginMenu("Simulation")) {
        if (!m_isPlaying) {
            if (ImGui::MenuItem("Play")) {
                m_isPlaying = true;

                // save current state to a temp scene file
                manager.saveToFile(m_tempSceneName);

                addDebugMessage("Simulation started.\n");
            }
        }
        else {
            if (ImGui::MenuItem("Stop")) {
                m_isPlaying = false;

                // load back the temp scene file to restore state
                manager.loadFromFile(m_tempSceneName);

                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;

                // remove temp scene file
                if (std::filesystem::exists(m_tempSceneName))
                    std::filesystem::remove(m_tempSceneName);

                addDebugMessage("Simulation stopped.\n");
            }
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::objListPanel(GameObjectManager& manager) {

    std::vector<const char*> names; // adjust max size

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    names.reserve(gameObjects.size());
    for (int i = 0; i < gameObjects.size(); ++i) {
        names.push_back(gameObjects[i]->getObjectName().c_str());
    }

    // show this if there are objects
    if (!names.empty()) {
        if (ImGui::ListBox("Objects", &m_SelectionState.selectedIndex, names.data(), static_cast<int>(names.size()), m_PanelState.maxObjects)) {
            m_SelectionState.selectedObject = gameObjects[m_SelectionState.selectedIndex];
        }
    }
    else {
        m_SelectionState.selectedObject = nullptr;
        ImGui::Text("No GameObjects available");
    }
}

void UI::objPropertyPanel(GameObjectManager& manager) {
    if (m_SelectionState.selectedObject) {

        /* ------------------- Edit Object Name ------------------- */
        std::string& objName = m_SelectionState.selectedObject->getObjectName();
        char buffer[128];
        strncpy(buffer, objName.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Name");
        ImGui::SameLine();
        if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
            std::string newObjName = buffer;
            if (manager.renameGameObject(m_SelectionState.selectedObject, newObjName)) {
                addDebugMessage("Object renamed to " + newObjName + "\n");
            }
            else {
                addDebugMessage("Failed to rename: name already exists\n");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ------------------- Object Transformation ------------------- */
        Transform* transform = m_SelectionState.selectedObject->getComponent<Transform>();
        if (ImGui::CollapsingHeader("Transform")) {

            ImGui::DragFloat("X", &transform->x);
            ImGui::DragFloat("Y", &transform->y);
            ImGui::DragFloat("Z", &transform->z);

            /*addDebugMessage("Transform updated: Position(" +
                std::to_string(transform->x) + ", " +
                std::to_string(transform->y) + ", " +
                std::to_string(transform->z) + ")\n");*/
        }

        /* ------------------- Object Rotation ------------------- */
        if (ImGui::CollapsingHeader("Rotation")) {
            ImGui::DragFloat("Angle", &transform->rotation);

            /*addDebugMessage("Transform updated: Rotation(" +
                std::to_string(transform->rotation) + ")\n");*/
        }

        /* ------------------- Ovject Scale ------------------- */
        if (ImGui::CollapsingHeader("Scale")) {
            ImGui::Checkbox("Aspect Ratio (X only)", &m_SelectionState.aspectRatioLock);

            if (m_SelectionState.aspectRatioLock && !m_SelectionState.ratioSet) {
                m_SelectionState.ratioY = transform->scaleY / transform->scaleX;
                m_SelectionState.ratioZ = transform->scaleZ / transform->scaleX;
                m_SelectionState.ratioSet = true;
            }

            if (!m_SelectionState.aspectRatioLock) m_SelectionState.ratioSet = false;

            ImGui::DragFloat("X1", &transform->scaleX);
            ImGui::DragFloat("Y1", &transform->scaleY);
            ImGui::DragFloat("Z1", &transform->scaleZ);

            if (m_SelectionState.aspectRatioLock && transform->scaleX != 0.0f) {
                transform->scaleY = transform->scaleX * m_SelectionState.ratioY;
                transform->scaleZ = transform->scaleX * m_SelectionState.ratioZ;
            }

            /*addDebugMessage("Transform updated: Position(" +
                std::to_string(transform->scaleX) + ", " +
                std::to_string(transform->scaleY) + ", " +
                std::to_string(transform->scaleZ) + ")\n");*/
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Change Texture ------------------- */
        // drag and drop received from asset window
        Render* render = m_SelectionState.selectedObject->getComponent<Render>();
        if (render && ImGui::CollapsingHeader("Render")) {
            ImGui::Text("Texture");
            ImGui::SameLine();

            std::string texName = render->texFile.empty() ? "(Drop new texture here)" : render->texFile;
            ImGui::Button(texName.c_str());

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                    const char* droppedPath = (const char*)payload->Data;
                    render->texFile = droppedPath;
                    render->hasTex = true;
                    render->texChanged = true;
                    addDebugMessage("Changed texture of " + m_SelectionState.selectedObject->getObjectName() + " to " + render->texFile + "\n");
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

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Delete Object ------------------- */
        std::string name = m_SelectionState.selectedObject->getObjectName();
        // do not show "DELETE" button for "PLAYER" IT WILL CRASH
        if (name != "player") {
            if (ImGui::Button("Delete Object")) {
                ImGui::OpenPopup("Confirm Delete");
            }
        }

        // delete confirm popup
        if (ImGui::BeginPopupModal("Confirm Delete", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete this object?");

            if (ImGui::Button("Yes")) {

                manager.deleteGameObject(m_SelectionState.selectedObject);

                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;

                addDebugMessage("Object " + name + " deleted\n");

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

    }
    else {
        ImGui::Text("No object selected");
    }
}

void UI::objInspectWindow(GameObjectManager& manager) {
    ImGui::Begin("Object Properties Editor");

    // Left panel - object list
    ImGui::BeginChild("Object List", ImVec2(m_PanelState.objListWidth, 0));
    objListPanel(manager);
    ImGui::EndChild();

    ImGui::SameLine();

    // splitter that allow resizing
    ImGui::InvisibleButton("splitter", ImVec2(m_PanelState.splitterSize, m_PanelState.objListHeight));
    if (ImGui::IsItemActive()) {
        m_PanelState.objListWidth += InputHandler::getMouseDelta().x;
        m_PanelState.objListWidth = std::clamp(m_PanelState.objListWidth, m_PanelState.objListMinWidth, m_PanelState.objListMaxWidth);
    }
    // draw the vertical line to indicate the splitter
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 splitterMin = ImGui::GetItemRectMin();
    ImVec2 splitterMax = ImGui::GetItemRectMax();
    draw_list->AddLine(
        ImVec2(splitterMin.x + m_PanelState.splitterSize * 0.5f, splitterMin.y),
        ImVec2(splitterMin.x + m_PanelState.splitterSize * 0.5f, splitterMax.y),
        IM_COL32(100, 100, 100, 255), 1.0f
    );

    ImGui::SameLine();

    // Right panel — inspector
    ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
    objPropertyPanel(manager);
    ImGui::EndChild();

    ImGui::End();
}

void UI::loadAssetsFromDirectory() {
    if (assetsChangedFlag == false) return; // only reload if flagged
    m_assets.clear(); // clear existing assets before loading new ones

    // iterate over all files and folders in this directory and push all files to m_assets (flat list)
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_regular_file()) {
            Asset asset;
            asset.name = entry.path().filename().string();
            asset.path = entry.path().string();
            asset.folder = entry.path().parent_path().string(); // folder in assets that contains this file, for future filtering

            m_assets.push_back(asset);
        }
    }

    assetsChangedFlag = false;
    addDebugMessage("Assets loaded from " + m_AssetBrowserState.currentFolder + "\n");
}

void UI::assetWindow(GameObjectManager& manager) {
    ImGui::Begin("Assets");

    // refresh to update assets from the actual asset folder
    // not really needed since we have auto refresh, but still good to have
    if (ImGui::Button("Refresh")) {
        assetsChangedFlag = true;
        loadAssetsFromDirectory();
    }

    // auto refresh when new files added to the asset folder (in project dir)
    std::filesystem::file_time_type current = std::filesystem::last_write_time(m_AssetBrowserState.currentFolder);
    if (current != m_AssetBrowserState.lastRefresh) {
        m_AssetBrowserState.lastRefresh = current;
        assetsChangedFlag = true; // to update assets
    }

    ImGui::Separator();

    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    int index{};
    for (const Asset& asset : m_assets) {
        // skip temp scene file (for play/stop)
        std::string fullPath = asset.folder + "/" + asset.name;
        if (fullPath == m_tempSceneName) continue;

        ImGui::PushID(index++);

        // display button and the name of the file
        ImGui::BeginGroup();

        // can consider changing to image in the future
        ImGui::Button(asset.name.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize));


        // double click logic
        if (ImGui::IsItemHovered() && InputHandler::isMouseLeftDoubleClicked()) {
            // if it's a scene file, load the scene
            if (asset.folder == m_AssetBrowserState.currentFolder + "Scene") {
                // load the new scene
                manager.loadFromFile(fullPath);

                // set current scene name
                m_currentSceneName = fullPath;

                // clear the current selection
                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;

                // for debugging
                addDebugMessage("Loaded scene: " + asset.name + "\n");
            }
        }

        // make sure this text wraps if it's too long
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
        ImGui::Text("%s", asset.name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        // calculate whether the next button should be displayed in the same line or next line based on this window size
        float lastButtonX2 = ImGui::GetItemRectMax().x;
        float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
        if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();

        // drag a texture to inspector window to change texture of selected object
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ASSET", asset.path.c_str(), asset.path.size() + 1);
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }


        ImGui::PopID();
    }

    ImGui::End();

}

void UI::sceneWindow(GLuint texture, float aspectRatio) {

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

    ImGui::EndChild();


    m_SceneWindowState.scenePos = ImGui::GetItemRectMin();
    m_SceneWindowState.sceneSize = ImGui::GetItemRectSize();

    ImGui::End();
}

/*
Future implementation (jic i forget)
- Add snapping to grid or axis (e.g. hold Shift for 1-unit steps)

- Highlight selected object while dragging

- Undo/redo
*/
void UI::objPicking(GameObjectManager& manager) {
    if (!InputHandler::isMouseLeftClicked()) return;

    Vector2D mousePos = InputHandler::getMousePositionInImGuiViewport(m_SceneWindowState.scenePos, m_SceneWindowState.sceneSize);

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    GameObject* selected = nullptr;

    for (auto* obj : gameObjects) {
        // dont let user click and select platform
        if (obj->getObjectName() == "platform") continue;

        Transform* transform = obj->getComponent<Transform>();
        if (!transform) continue;

        Collision::AABB aabb = Collision::getObjectAABB(transform);
        Collision::Circle mousePoint(mousePos, 0.2f);

        // check if mouse point intersects with the game obj
        if (Collision::CollisionIntersection_CircleAABB_Static(mousePoint, aabb)) {
            selected = obj;
        }
    }

    // if something is selected, update selection state
    if (selected) {
        m_SelectionState.selectedObject = selected;
        m_SelectionState.draggedObject = selected;

        auto iterator = std::find(gameObjects.begin(), gameObjects.end(), selected);
        if (iterator != gameObjects.end()) {
            m_SelectionState.selectedIndex = static_cast<int>(std::distance(gameObjects.begin(), iterator));
        }

        addDebugMessage(selected->getObjectName() + " selected");
    }
    else {
        // if mouse click somewhere else, deselect current dragged object
        m_SelectionState.draggedObject = nullptr;
    }
}

void UI::objDragging() const {
    // early return if no obj is selected
    GameObject* obj = m_SelectionState.draggedObject;
    if (!obj) return;

    // early return if mouse is not dragging
    if (!InputHandler::isMouseDragging()) return;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform) return;

    // change current object position based on mouse delta
    Vector2D mousePos = InputHandler::getMouseDeltaWorldInViewport(m_SceneWindowState.sceneSize);
    transform->x += mousePos.x;
    transform->y += mousePos.y;
}