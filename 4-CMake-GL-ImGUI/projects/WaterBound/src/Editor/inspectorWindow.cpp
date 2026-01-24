/* Start Header ************************************************************************/
/*!
\file       inspectorWindow.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of different components in the
            Inspector. Magic vector is used here to allow adding of components not
            owe by the object. Gizmo toggling with mouse is also included here.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

InspectorWindow::InspectorWindow(Editor::ObjSelectionState& Ostate, Editor::GizmoState& gState) : 
    m_objSelectionState(Ostate),
    m_gizmoState(gState) {}

void InspectorWindow::render(GameObjectManager& manager) {
    ImGui::Begin("Inspector");

    GameObject* selected = nullptr;
    if (m_objSelectionState.selectedObject) selected = m_objSelectionState.selectedObject;
    else if (m_objSelectionState.selectedPrefab) {
        selected = m_objSelectionState.selectedPrefab.get();

        if (ImGui::Button("Save")) {
            const std::string& prefabID = selected->getObjectPrefabID();

            if (PrefabManager::Instance().savePrefab(selected)) {
                int count = PrefabManager::Instance().applyToAllInstances(prefabID, manager);
                /*manager.loadScene(JsonIO::runtimeScenePath(Editor::sceneState.currentSceneName));*/
                DebugLog::addMessage("Applied prefab changes to " + std::to_string(count) + " instances\n");
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
        /*if (selected == m_objSelectionState.selectedObject) {*/
            std::string& objName = selected->getObjectName();
            char buffer[128];
            strncpy(buffer, objName.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            ImGui::Text("Name");
            ImGui::SameLine();

            // ENTER to apply name change
            if (ImGui::InputText("##Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                std::string oldObjName = objName;
                std::string newObjName = buffer;

                if (selected == m_objSelectionState.selectedObject) {
                    if (manager.renameGameObject(m_objSelectionState.selectedObject, newObjName)) {
                        // update obj new name jic redo/undo
                        UndoRedoManager::Instance().updateObjName(oldObjName, newObjName);

                        DebugLog::addMessage("Object renamed to " + newObjName + "\n");
                    }
                    else {
                        DebugLog::addMessage("Failed to rename: name already exists\n");
                    }
                }
                else {
                    const std::string& prefabID = selected->getObjectPrefabID();
                    PrefabManager::Instance().getPrefabName(prefabID) = newObjName;
                    selected->getObjectName() = newObjName;
                    //PrefabManager::Instance().savePrefabRegistry();

                    DebugLog::addMessage("Prefab renamed to " + newObjName + "\n");
                }
            }
            ImGui::Text("ENTER to apply");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Object Transformation ------------------- */
        if (selected->hasComponent<Transform>()) {
            if (selected == m_objSelectionState.selectedObject) renderGizmoCtrl();


            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            renderTransform(selected, manager);
        }
        /* ------------------- END ------------------- */

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ------------------- Render ------------------- */
        if (selected->hasComponent<Render>()) {
            renderRender(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- State Machine ------------------- */
        if (selected->hasComponent<StateMachine>()) {
            renderStateMachine(selected);
            // for animation/audio(maybe)

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Collision ------------------- */
        if (selected->hasComponent<CollisionInfo>()) {
            renderCollision(selected, manager);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Input Control ------------------- */
        if (selected->hasComponent<Input>()) {
            renderInput(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Physics ------------------- */
        if (selected->hasComponent<Physics>()) {
            renderPhysics(selected, selected->hasComponent<Input>());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Font ------------------- */
        if (selected->hasComponent<FontComponent>()) {
            renderFont(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Font ------------------- */
        if (selected->hasComponent<AudioComponent>()) {
            renderAudio(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Font ------------------- */
        if (selected->hasComponent<TileMap>()) {
            renderTileMap(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Lua ------------------- */
        if (selected->hasComponent<LuaScript>()) {
            renderLua(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Visual LC ------------------- */
        if (selected->hasComponent<VisualLC>()) {
            renderVisualLogic(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Add Component ------------------- */
        addComponent(selected);
        /* ------------------- END ------------------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }
    else {
        ImGui::Text("No object selected");

        m_objSelectionState.selectedObject = nullptr;
        m_objSelectionState.draggedObject = nullptr;
        m_objSelectionState.selectedPrefab = nullptr;
        m_objSelectionState.selectedIndex = -1;
    }

    ImGui::End();

    wrongFileTypePopup(Editor::popupState.filename);
}

void InspectorWindow::addComponent(GameObject* selected) {
    //if (!ImGui::CollapsingHeader("Add Component")) return;

    ImGui::BeginChild("AddComponentSection", ImVec2(0, 400), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("Add Component");
    ImGui::Separator();

    static char search[64] = "";
    ImGui::InputTextWithHint("##search", "Search components...", search, sizeof(search));

    for (const Editor::ComponentEntry& comp : Editor::componentRegistry) {
        // skip if selected obj alr has this component
        if (comp.hasComp(selected)) continue;

        // search
        if (strlen(search) > 0) {
            std::string compName = comp.name;
            std::string find = search;

            std::transform(compName.begin(), compName.end(), compName.begin(), ::tolower);
            std::transform(find.begin(), find.end(), find.begin(), ::tolower);

            if (compName.find(find) == std::string::npos) continue;
        }

        // draw component selectable
        if (ImGui::Selectable(comp.name.c_str())) {
            comp.addComp(selected);
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::EndChild();
}

void InspectorWindow::renderGizmoCtrl() {
    if (ImGui::TreeNode("Gizmo Control")) {
        // toggle gizmo
        if (ImGui::RadioButton("Move (W)", m_gizmoState.currentOp == Editor::GIZMO_TRANSLATE)) {
            m_gizmoState.currentOp = Editor::GIZMO_TRANSLATE;
        }

        if (ImGui::RadioButton("Rotate (E)", m_gizmoState.currentOp == Editor::GIZMO_ROTATE)) {
            m_gizmoState.currentOp = Editor::GIZMO_ROTATE;
        }

        if (ImGui::RadioButton("Scale (R)", m_gizmoState.currentOp == Editor::GIZMO_SCALE)) {
            m_gizmoState.currentOp = Editor::GIZMO_SCALE;
        }

        // snapping
        if (ImGui::TreeNode("Snap")) {
            ImGui::Checkbox("Enable", &m_gizmoState.useSnap);
            if (m_gizmoState.useSnap) {
                // if rotating, show the snap value (only 1 value)
                if (m_gizmoState.currentOp == Editor::GIZMO_ROTATE) {
                    if (ImGui::DragFloat("##Snap Angle", &m_gizmoState.snapAngle)) {
                        // clamp to 1�180 degrees
                        m_gizmoState.snapAngle = glm::clamp(m_gizmoState.snapAngle, 1.f, 365.f);
                    }
                }
                else {
                    if (ImGui::DragFloat3("###Snap XYZ", m_gizmoState.snapValues)) {
                        for (int i = 0; i < 3; i++)
                            m_gizmoState.snapValues[i] = glm::max(m_gizmoState.snapValues[i], 0.01f);
                    }
                }
            }
            ImGui::TreePop();
        }

        // grid
        /*if (ImGui::TreeNode("Grid")) {
            ImGui::Checkbox("Enable", &m_gizmoState.showGrid);
            if (m_gizmoState.showGrid) {
                ImGui::Indent();
                if (ImGui::InputInt("Grid Spacing", &m_gizmoState.gridSpacing))
                    m_gizmoState.gridSpacing = std::max(m_gizmoState.gridSpacing, 0);
                ImGui::Unindent();
            }
            ImGui::TreePop();
        }*/

        ImGui::TreePop();
    }
}

void InspectorWindow::renderTransform(GameObject* selected, GameObjectManager& manager) {
    //Transform* transform, CollisionInfo* collision
    Transform* transform = selected->getComponent<Transform>();
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();

    if (ImGui::CollapsingHeader("Transform")) {

        if (ImGui::TreeNode("Position")) {
			bool posActive = false;

            if (ImGui::DragFloat("X", &transform->x, 0.1f)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }
            }
			posActive |= ImGui::IsItemActive();

            if (ImGui::DragFloat("Y", &transform->y, 0.1f)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }
            }
            posActive |= ImGui::IsItemActive();

            // set z to stay within the editor camera (-1.f <= z < 1.f)
            glm::mat4& proj = renderer::editorCam.proj;
            float m22 = proj[2][2];
            float m32 = proj[3][2];
            float near1 = (m32 - 1.f) / m22;
            float far1 = (m32 + 1.f) / m22;

            if (ImGui::DragFloat("Z", &transform->z, 0.1f, far1, near1)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }
            }
            posActive |= ImGui::IsItemActive();

            if (m_isEditingPos && !posActive) {
                UndoRedoManager::Instance().endTransformEdit(manager, m_transformEditObject);
                m_isEditingPos = false;
            }

            ImGui::TreePop();
        }

        /* ------------------- Object Rotation ------------------- */
        if (ImGui::TreeNode("Rotation")) {
            if (ImGui::DragFloat("Angle", &transform->rotation)) {
                transform->rotation = std::clamp(transform->rotation, 0.f, 360.f);

                if (!m_isEditingRot) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingRot = true;
                    m_transformEditObject = selected;
                }
            }

            bool rotationActive = ImGui::IsItemActive();

            if (m_isEditingRot && !rotationActive) {
                UndoRedoManager::Instance().endTransformEdit(manager, m_transformEditObject);
                m_isEditingRot = false;
            }

            ImGui::TreePop();
        }

        /* ------------------- Object Scale ------------------- */
        if (ImGui::TreeNode("Scale")) {
            bool scaleActive = false;

            ImGui::Checkbox("Aspect Ratio", &m_objSelectionState.aspectRatioLock);

            if (m_objSelectionState.aspectRatioLock && !m_objSelectionState.ratioSet) {
                m_objSelectionState.ratioY = transform->scaleY / transform->scaleX;
                m_objSelectionState.ratioZ = transform->scaleZ / transform->scaleX;
                m_objSelectionState.ratioSet = true;
            }

            if (!m_objSelectionState.aspectRatioLock) m_objSelectionState.ratioSet = false;

            // clamp scale to be non-negative
            if (ImGui::DragFloat("X1", &transform->scaleX, 0.1f, 0.f)) {
                transform->scaleX = std::max(transform->scaleX, 0.f);

                if (!m_isEditingScale) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingScale = true;
                    m_transformEditObject = selected;
                }
            }
			scaleActive |= ImGui::IsItemActive();

            ImGui::BeginDisabled(m_objSelectionState.aspectRatioLock);
            if (ImGui::DragFloat("Y1", &transform->scaleY, 0.1f, 0.f)) {
                transform->scaleY = std::max(transform->scaleY, 0.f);

                if (!m_isEditingScale) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingScale = true;
                    m_transformEditObject = selected;
                }
            }
			scaleActive |= ImGui::IsItemActive();

            if (ImGui::DragFloat("Z1", &transform->scaleZ, 0.1f, 0.f)) {
                transform->scaleZ = std::max(transform->scaleZ, 0.f);


                if (!m_isEditingScale) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingScale = true;
                    m_transformEditObject = selected;
                }
            }
			scaleActive |= ImGui::IsItemActive();

            ImGui::EndDisabled();

            if (m_objSelectionState.aspectRatioLock && transform->scaleX != 0.0f) {
                transform->scaleY = transform->scaleX * m_objSelectionState.ratioY;
                transform->scaleZ = transform->scaleX * m_objSelectionState.ratioZ;
            }

            // if collision and its set to auto-fit scale, adjust the collider size also
            if (collision && collision->autoFitScale) {
                // if collider is circle, use the largest of x/y scale 
                if (collision->colliderType == shape::circle) {
                    float largest = std::max(transform->scaleX, transform->scaleY);
                    collision->colliderSize.x = largest;
                    collision->colliderSize.y = largest;
                }
                // for box collider, fit width and height
                else {
                    collision->colliderSize.x = transform->scaleX;
                    collision->colliderSize.y = transform->scaleY;
                }
            }

            if (m_isEditingScale && !scaleActive) {
                UndoRedoManager::Instance().endTransformEdit(manager, m_transformEditObject);
                m_isEditingScale = false;
            }

            ImGui::TreePop();
        }
    }
}

void InspectorWindow::renderRender( GameObject* selected) {
    Render* render = selected->getComponent<Render>();
    //Animation* animation = selected->getComponent<Animation>();
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();

    if (ImGui::CollapsingHeader("Render")) {

        if (ImGui::TreeNode("Texture")) {

            std::string texName = render->texFile.empty() ? "(Drop new texture here)" : render->texFile;
            ImGui::Button(texName.c_str());

            /* -------------- Drag and Drop From Asset Browser to Change Texture ------------- */
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                    const char* droppedPath = (const char*)payload->Data;
                    std::string path = droppedPath;

                    const std::string debugPrefix = "Debug/";

                    std::filesystem::path fsPath(path);
                    if (fsPath.string().find(debugPrefix) == 0) {
                        fsPath = fsPath.string().substr(debugPrefix.size());
                    }

                    // extract file extension
                    std::string ext = fsPath.extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    // only accept if dropped file is a png/jpg/jpeg
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        render->texFile = path;
                        render->hasTex = true;
                        render->texChanged = true;
                        DebugLog::addMessage("Changed texture of " + selected->getObjectName() + " to " + render->texFile + "\n");
                    }
                    else {
                        // texture file type error pop up
                        Editor::popupState.textureFileTypePopup = true;
                        Editor::popupState.filename = fsPath.filename().string();

                        // invalid texture format -> don't change anything
                        DebugLog::addMessage("Invalid texture format : " + path + "\n");
                    }

                }
                ImGui::EndDragDropTarget();
            }
            
            if (render->hasTex) {
                ImGui::SameLine();
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("Clear Texture")) {
                    glDeleteTextures(1, &render->texHDL);
                    render->texHDL = 0;
                    render->hasTex = false;
                    render->texFile.clear();
                    render->texChanged = true;
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Shape")) {
            const char* shapes[] = { "Rectangle", "Circle" };
            int current = (render->modelRef.shape == shape::square) ? 0 : 1;

            if (ImGui::Combo("##Shape", &current, shapes, IM_ARRAYSIZE(shapes))) {
                if (current == 0)
                    render->modelRef.shape = shape::square;
                else
                    render->modelRef.shape = shape::circle;

                if (collision && collision->autoFitScale) {
                    collision->colliderType = render->modelRef.shape;
                }
            }

            ImGui::TreePop();
        } 

        if (ImGui::TreeNode("Colour")) {
            // cannot edit colour if obj has a texture
            ImGui::BeginDisabled(render->hasTex);

            ImGui::ColorEdit3("##Colour", &render->clr.x); // pass pointer of the first float cuz imgui want that

            ImGui::EndDisabled();
            ImGui::TreePop();
        }
        
     
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove Render", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<Render>();
            DebugLog::addMessage("Removed Render Component");
        }
    }
}

void InspectorWindow::renderStateMachine(GameObject* selected) {
    Render* render = selected->getComponent<Render>();
    Animation* animation = selected->getComponent<Animation>();

    if (ImGui::CollapsingHeader("State Machine")) {
        /* ---------- Animation ---------- */
        if (ImGui::TreeNode("Animation")) {
            // must have render component to animate
            if (!render) {
                ImGui::TextWrapped("Animation requires a Render component.");
            }
            else if (!animation) {
                ImGui::Spacing();
                if (ImGui::Selectable("Add Animation")) {
                    selected->addComponent<Animation>();
                }
            }
            else if (!animation->animState.empty()) {
                ImGui::Checkbox("Is animated", &render->hasAnimation);

                ImGui::Separator();

                ImGui::BeginDisabled(!render->hasAnimation);

                static int selectedStateIndex = 0;
                const std::vector<std::string>& stateNames = JsonIO::stateNames;

                // Dropdown to select state
                if (ImGui::BeginCombo("State", stateNames[selectedStateIndex].c_str())) {
                    for (int i = 0; i < stateNames.size(); ++i) {
                        bool isSelected = (selectedStateIndex == i);
                        if (ImGui::Selectable(stateNames[i].c_str(), isSelected)) {
                            selectedStateIndex = i;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Separator();

                // Edit animation for the selected state
                if (selectedStateIndex >= 0 && selectedStateIndex < animation->animState.size()) {
                    AnimateState& currentState = animation->animState[selectedStateIndex];
                                   
                    // Texture file button with drag and drop
                    std::string texName = currentState.texFile.empty() ? "(Drop new texture here)" : currentState.texFile;
                    ImGui::Button(texName.c_str(), ImVec2(-1, 0));

                    /* ------- Drag and Drop From Asset Browser to Change Texture ------ */
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                            const char* droppedPath = (const char*)payload->Data;
                            std::string path = droppedPath;

                            const std::string debugPrefix = "Debug/";
                            std::filesystem::path fsPath(path);
                            if (fsPath.string().find(debugPrefix) == 0) {
                                fsPath = fsPath.string().substr(debugPrefix.size());
                            }

                            // extract file extension
                            std::string ext = fsPath.extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                            // only accept if dropped file is a png/jpg/jpeg
                            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                                currentState.texFile = path;
                                currentState.texChanged = true;
                                DebugLog::addMessage("Changed texture for " + stateNames[selectedStateIndex] + " state to " + path + "\n");
                            }
                            else {
                                // texture file type error pop up
                                Editor::popupState.textureFileTypePopup = true;
                                Editor::popupState.filename = fsPath.filename().string();
                                // invalid texture format -> don't change anything
                                DebugLog::addMessage("Invalid texture format : " + path + "\n");
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::BeginDisabled(currentState.texFile.empty());

                    ImGui::SameLine();
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (!currentState.texFile.empty()){
                        if (ImGui::Button("Clear Texture")) {
                            glDeleteTextures(1, &currentState.texHDL);
                            currentState.texHDL = 0;
                            currentState.texFile.clear();
                            currentState.texChanged = true;
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Checkbox("Loop (WIP)", &currentState.loop);

                    if (ImGui::InputInt("Total Columns", &currentState.totalColumn))
                        currentState.totalColumn = std::max(currentState.totalColumn, 1);

                    if (ImGui::InputInt("Total Rows", &currentState.totalRow))
                        currentState.totalRow = std::max(currentState.totalRow, 1);

                    int tempInitCol = (int)currentState.initialFrame.x;
                    int tempInitRow = (int)currentState.initialFrame.y;
                    int tempLastCol = (int)currentState.lastFrame.x;
                    int tempLastRow = (int)currentState.lastFrame.y;

                    if (ImGui::InputInt("Initial Column", &tempInitCol))
                        currentState.initialFrame.x = (float)tempInitCol;
                    if (ImGui::InputInt("Initial Row", &tempInitRow))
                        currentState.initialFrame.y = (float)tempInitRow;
                    if (ImGui::InputInt("Last Column", &tempLastCol))
                        currentState.lastFrame.x = (float)tempLastCol;
                    if (ImGui::InputInt("Last Row", &tempLastRow))
                        currentState.lastFrame.y = (float)tempLastRow;

                    if(ImGui::InputFloat("Frame Time", &currentState.frameTime, 0.001f))
                        currentState.frameTime = std::max(currentState.frameTime, 0.001f);

                    ImGui::EndDisabled();
                }

                ImGui::EndDisabled();

                ImGui::Separator();
                if (ImGui::Button("Remove Animation", ImVec2(-1, 0))) {
                    selected->removeComponent<Animation>();
                    DebugLog::addMessage("Removed Animation Component");
                }

            }
            else {

                if (ImGui::Button("Remove Animation", ImVec2(-1, 0))) {
                    // add a delete component func here
                    selected->removeComponent<Animation>();
                    DebugLog::addMessage("Removed Animation Component");
                }
            }
                        
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove State Machine", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<StateMachine>();
            DebugLog::addMessage("Removed State Machine");
        }

    }
}

void InspectorWindow::renderCollision( GameObject* selected, GameObjectManager& manager) {
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();
    Transform* transform = selected->getComponent<Transform>();
    Render* render = selected->getComponent<Render>();

    if (ImGui::CollapsingHeader("Collision")) {
        {
            // checked means this object can collide with obj in the same layer
            ImGui::Checkbox("Enable", &collision->collisionFlag);
            
            // select layer
            int layer = selected->getLayer();
            if (ImGui::InputInt("Layer", &layer)) {
                if (layer < 0) layer = 0;
                
                if (m_objSelectionState.selectedPrefab) {
                    selected->setLayer(layer);
                }
                else if (manager.assignObjectToLayer(selected, layer)) {
                    DebugLog::addMessage(selected->getObjectName() + " changed layer to " + std::to_string(selected->getLayer()));

                }
                else {
                    DebugLog::addMessage(selected->getObjectName() + " failed to change layer!" );
                }

            }

            std::string currentTag = collision->collisionTag.empty() ? "default" : collision->collisionTag;

            if (ImGui::BeginCombo("Collision Tag", currentTag.c_str())) {
                const std::vector<std::string>& collisionTags = GlobalRegistry::Instance().getCollisionTags();

                for (const std::string& tag : collisionTags) {
                    bool isSelected = (currentTag == tag);

                    if (ImGui::Selectable(tag.c_str(), isSelected)) {
                        collision->collisionTag = tag;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            /*if (ImGui::BeginCombo("Collide with", filterType.c_str())) {
                const char* options[] = { "any", "name", "tag" };
                const char* filterLabels[] = { "Any Object", "Object Name", "Collision Tag" };

                for (const char* option : options) {
                    bool isSelected = (filterType == option);

                    if (ImGui::Selectable(option, isSelected)) {
                        node.parameters["colFilterType"] = option;

                        if (!isSelected) {
                            node.parameters["colFilterValue"] = "";
                        }
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }*/

               /* ImGui::EndCombo();
            }*/

        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::TreeNode("Collider")) {
            if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = true;

            // cannot edit if obj cannot collide
            ImGui::BeginDisabled(!collision->collisionFlag);
            if (ImGui::Checkbox("Auto-fit Scale", &collision->autoFitScale)) {
                if (collision->autoFitScale && transform) {
                    collision->colliderSize.x = transform->scaleX;
                    collision->colliderSize.y = transform->scaleY;
                }

                if (collision->autoFitScale && render) {
                    collision->colliderType = render->modelRef.shape;
                }
            }

            CollisionResponseMode mode = collision->collisionRes;
            const char* modes[] = { "Static", "Pushable" };
            int current = static_cast<int>(mode);

            if (ImGui::Combo("Collision Response", &current, modes, IM_ARRAYSIZE(modes))) {
                collision->collisionRes = static_cast<CollisionResponseMode>(current);
            }

            ImGui::EndDisabled();

            // cannot edit if obj cannot collide or auto-fit collider size with obj size is on
            bool editable = collision->collisionFlag && !collision->autoFitScale;
            ImGui::BeginDisabled(!editable);

            shape colliderShape = collision->colliderType;
            const char* shapes[] = { "Box", "Circle" };
            current = static_cast<int>(colliderShape);

            if (ImGui::Combo("Shape", &current, shapes, IM_ARRAYSIZE(shapes))) {
                collision->colliderType = static_cast<shape>(current);
            }
                                
            if (collision->colliderType == shape::circle) {
                if (ImGui::InputFloat("Diamater", &collision->colliderSize.x, 1.f)) {
                    collision->colliderSize.x = std::max(collision->colliderSize.x, 0.f);
                    collision->colliderSize.y = collision->colliderSize.y;
                }
            }
            else {
                if (ImGui::InputFloat("Width", &collision->colliderSize.x, 1.f))
                    collision->colliderSize.x = std::max(collision->colliderSize.x, 0.f);

                if (ImGui::InputFloat("Height", &collision->colliderSize.y, 1.f))
                    collision->colliderSize.y = std::max(collision->colliderSize.y, 0.f);
            }

            ImGui::EndDisabled();

            ImGui::TreePop();
        }
        else {
            if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = false;
        }

        if (ImGui::Button("Remove Collision", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<CollisionInfo>();
            DebugLog::addMessage("Removed Collision Component");
        }
    }
    else {
        if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = false;
    }
}

void InspectorWindow::renderInput(GameObject* selected) {
    if (ImGui::CollapsingHeader("Input Control")) {
        ImGui::TextWrapped("This object can be controlled by keyboard input");

        if (ImGui::Button("Remove Input", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<Input>();
            DebugLog::addMessage("Removed Input Component");
        }
    }
}

void InspectorWindow::renderPhysics(GameObject* selected, bool canMove) {
    Physics* physic = selected->getComponent<Physics>();

    if (ImGui::CollapsingHeader("Physics")) {
        ImGui::Checkbox("Is Trigger", &physic->physicsFlag);

        ImGui::Spacing();
        ImGui::Separator();
       

        // only show movement data if object is controlable via input
        if (ImGui::TreeNode("Input Movement Control")) {
            if (canMove) {
                if (ImGui::TreeNode("Movement")) {
                    if(ImGui::InputFloat("Speed", &physic->moveSpeed, 1.f))
                        physic->moveSpeed = std::max(physic->moveSpeed, 0.f); // set the min speed to 0.f

                    if(ImGui::InputFloat("Jump Force", &physic->jumpForce, 1.f))
                        physic->jumpForce = std::max(physic->jumpForce, 0.f); // set the min jump force to 0.f

                    if(ImGui::InputFloat("Mass", &physic->dynamics.mass, 1.f))
                        physic->dynamics.mass = std::max(physic->dynamics.mass, 0.f); // 0.f mass means static obj (never move, e.g. wall)

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Environment")) {
                    if(ImGui::InputFloat("Damping", &physic->damping, 0.1f))
                        physic->damping = std::clamp(physic->damping, 0.f, 1.f); // damping within 0.1 to 1.f

                    ImGui::TreePop();
                }
            }
            else {
                ImGui::TextWrapped("Requires Input component to move");
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Water")) {
            ImGui::Checkbox("Buoyancy (we dh)", &physic->buoancy);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Debug Info")) {
            ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
                physic->dynamics.velocity.x,
                physic->dynamics.velocity.y,
                physic->dynamics.velocity.z);

            ImGui::Spacing();

            Vector2D vel2D{ physic->dynamics.velocity.x, physic->dynamics.velocity.y };
            float vel = Vec_Length(&vel2D);
            ImGui::Text("Speed: %.2f", vel);

            ImGui::Spacing();

            ImGui::Text("Acceleration: (%.2f, %.2f, %.2f)",
                physic->dynamics.acceleration.x,
                physic->dynamics.acceleration.y,
                physic->dynamics.acceleration.z);

            ImGui::Spacing();

            ImGui::Text("Inverse Mass: %.4f", physic->dynamics.inverseMass);

            ImGui::TreePop();
        }

        if (ImGui::Button("Remove Physics", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<Physics>();
            DebugLog::addMessage("Removed Physics Component");
        }
    }
}

void InspectorWindow::renderFont(GameObject* selected) {
    FontComponent* font = selected->getComponent<FontComponent>();

    if (ImGui::CollapsingHeader("Font")) {
        {
            // word to display
            std::string& objName = font->word;
            char buffer[128];
            strncpy(buffer, objName.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText("##Word", buffer, sizeof(buffer))) {
                font->word = buffer;
            }

            // size of the text
            if(ImGui::InputFloat("Size", &font->scale, 1.f))
                font->scale = std::max(font->scale, 0.f);

            // colour of the text
            ImGui::ColorEdit3("Colour", &font->clr.x);

            // font type of the text
            /* scuffed way to select font */
            static const char* fontNames[] = {
                "Orange Knight",
                "Arial",
                "Times New Roman"
            };

            ImGui::Combo("Font Type", &font->fontType, fontNames, IM_ARRAYSIZE(fontNames));
        }

        if (ImGui::Button("Remove Font", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<FontComponent>();
            DebugLog::addMessage("Removed Font Component");
        }
    }
}

void InspectorWindow::renderAudio(GameObject* selected) {
    AudioComponent* audio = selected->getComponent<AudioComponent>();

    if (ImGui::CollapsingHeader("Audio")) {

        // Add New Channel Section
        ImGui::Text("Add New Audio Channel");
        static char newChannelName[64] = "";
        ImGui::InputTextWithHint("##NewChannelName", "Channel name (e.g., footsteps)", newChannelName, sizeof(newChannelName));
        ImGui::SameLine();
        if (ImGui::Button("Add Channel")) {
            if (strlen(newChannelName) > 0) {
                std::string channelName = newChannelName;
                if (!audio->hasChannel(channelName)) {
                    audio->getOrCreateChannel(channelName);
                    DebugLog::addMessage("Added audio channel: " + channelName + "\n");
                    newChannelName[0] = '\0'; // Clear input
                }
                else {
                    DebugLog::addMessage("Channel '" + channelName + "' already exists!\n");
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Display All Channels
        if (audio->audioChannels.empty()) {
            ImGui::Text("No audio channels. Add one above.");
        }
        else {
            ImGui::Text("Audio Channels (%d)", (int)audio->audioChannels.size());

            // Track channels to remove (can't remove during iteration)
            static std::string channelToRemove;
            channelToRemove.clear();

            for (auto& [channelName, ch] : audio->audioChannels) {
                ImGui::PushID(channelName.c_str());

                // Channel header with status indicator
                std::string headerLabel = channelName;
                if (ch.state == AudioState::Playing) {
                    headerLabel += " [PLAYING]";
                }
                else if (ch.state == AudioState::Paused) {
                    headerLabel += " [PAUSED]";
                }

                bool channelOpen = ImGui::CollapsingHeader(headerLabel.c_str());

                // Show audio file button even when collapsed (for quick reference)
                if (!channelOpen) {
                    std::string fileName = ch.audioFile.empty() ? "(No audio)" : std::filesystem::path(ch.audioFile).filename().string();
                    ImGui::SameLine();
                    ImGui::Text("- %s", fileName.c_str());
                }

                if (channelOpen) {
                    ImGui::Indent();

                    // Audio File Selector
                    std::string audioName = ch.audioFile.empty() ? "(Drop audio here)" : ch.audioFile;
                    ImGui::Button(audioName.c_str(), ImVec2(-1, 0));

                    /* -------------- Drag and Drop From Asset Browser ------------- */
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                            const char* droppedPath = (const char*)payload->Data;
                            std::string path = droppedPath;

                            const std::string debugPrefix = "Debug/";
                            std::filesystem::path fsPath(path);
                            if (fsPath.string().find(debugPrefix) == 0) {
                                fsPath = fsPath.string().substr(debugPrefix.size());
                            }

                            // Extract file extension
                            std::string ext = fsPath.extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                            // Only accept audio files
                            if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") {
                                ch.audioFile = path;
                                DebugLog::addMessage("Changed audio of channel '" + channelName + "' to " + ch.audioFile + "\n");
                            }
                            else {
                                Editor::popupState.audioFileTypePopup = true;
                                Editor::popupState.filename = fsPath.filename().string();
                                DebugLog::addMessage("Unsupported audio format: " + path + "\n");
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (!ch.audioFile.empty()) {
                        ImGui::Spacing();
                        
                        // Manual Playback Controls
                        ImGui::Text("Playback Controls");

                        if (ch.channel && ch.state == AudioState::Playing) {
                            // Currently playing - show Stop button
                            if (ImGui::Button("Stop", ImVec2(-1, 0))) {
                                ch.isPendingStop = true;
                                DebugLog::addMessage("Manually stopped channel: " + channelName + "\n");
                            }
                        }
                        else if (!ch.audioFile.empty()) {
                            // Not playing but has audio file - show Play button
                            if (ImGui::Button("Play", ImVec2(-1, 0))) {
                                ch.isPendingPlay = true;
                                DebugLog::addMessage("Manually started channel: " + channelName + "\n");
                            }
                        }
                        else {
                            // No audio file assigned
                            ImGui::BeginDisabled();
                            ImGui::Button("Play (No Audio)", ImVec2(-1, 0));
                            ImGui::EndDisabled();
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Basic Controls - detect changes and stop sound if settings change
                        bool loopChanged = ImGui::Checkbox("Loop", &ch.loop);
                        bool playOnStartChanged = ImGui::Checkbox("Play On Start", &ch.playOnStart);
                        bool mutedChanged = ImGui::Checkbox("Muted", &ch.muted);
						ImGui::Checkbox("Spatial Audio", &ch.isSpatial);

                        // Stop currently playing sound if critical settings changed
                        if ((loopChanged || playOnStartChanged || mutedChanged) && ch.channel) {
                            ch.isPendingStop = true;
                            DebugLog::addMessage("Stopped audio in '" + channelName + "' due to setting change\n");
                        }

                        if (ch.isSpatial) {
                            ImGui::Indent();  // Indent to show hierarchy

                            if (ImGui::DragFloat("Min Distance", &ch.minDistance, 0.1f, 0.1f, 1000.0f)) {
                                ch.minDistance = std::clamp(ch.minDistance, 0.1f, 1000.0f);
                            }

                            if (ImGui::DragFloat("Max Distance", &ch.maxDistance, 1.0f, 1.0f, 10000.0f)) {
                                ch.maxDistance = std::clamp(ch.maxDistance, 1.0f, 10000.0f);
                            }

                            ImGui::Unindent();
                        }

                        ImGui::Spacing();

                        // Volume & Pitch Controls
                        if (ImGui::TreeNode("Volume & Pitch")) {
                            if (ImGui::DragFloat("Volume", &ch.volume, 0.01f, 0.0f, 1.0f)) {
                                ch.volume = std::clamp(ch.volume, 0.f, 1.f);
                            }

                            if (ImGui::DragFloat("Pitch", &ch.pitch, 0.01f, 0.1f, 2.0f)) {
                                ch.pitch = std::clamp(ch.pitch, 0.1f, 2.f);
                            }

                            ImGui::TreePop();
                        }

                        // Fade Controls
                        if (ImGui::TreeNode("Fade In/Out")) {
                            ImGui::Checkbox("Fade In On Start", &ch.fadeInOnStart);
                            if (ch.fadeInOnStart) {
                                ImGui::Indent();
                                if (ImGui::DragFloat("Fade In Duration", &ch.fadeInDuration, 0.01f, 0.0f, 10.0f)) {
                                    ch.fadeInDuration = std::clamp(ch.fadeInDuration, 0.f, 10.f);
                                }
                                ImGui::Unindent();
                            }

                            ImGui::Checkbox("Fade Out On Stop", &ch.fadeOutOnStop);
                            if (ch.fadeOutOnStop) {
                                ImGui::Indent();
                                if (ImGui::DragFloat("Fade Out Duration", &ch.fadeOutDuration, 0.01f, 0.0f, 10.0f)) {
                                    ch.fadeOutDuration = std::clamp(ch.fadeOutDuration, 0.f, 10.f);
                                }
                                ImGui::Unindent();
                            }

                            ImGui::TreePop();
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Remove Sound (clears file but keeps channel)
                        if (ImGui::Button("Clear Audio File", ImVec2(-1, 0))) {
                            ch.audioFile = "";
                            if (ch.channel) {
                                ch.channel->stop();
                                ch.channel = nullptr;
                            }
                            DebugLog::addMessage("Cleared audio from channel: " + channelName + "\n");
                        }
                    }

                    ImGui::Spacing();

                    // Remove Channel Button
                    if (ImGui::Button(("Remove Channel: " + channelName).c_str(), ImVec2(-1, 0))) {
                        channelToRemove = channelName;
                    }

                    ImGui::Unindent();
                }

                ImGui::PopID();
            }

            // Remove channel if marked for removal
            if (!channelToRemove.empty()) {
                audio->removeChannel(channelToRemove);
                DebugLog::addMessage("Removed audio channel: " + channelToRemove + "\n");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Remove Entire Component
        if (ImGui::Button("Remove Audio Component", ImVec2(-1, 0))) {
            selected->removeComponent<AudioComponent>();
            DebugLog::addMessage("Removed Audio Component");
        }
    }
}

void InspectorWindow::renderTileMap(GameObject* selected)
{
    TileMap* tm = selected->getComponent<TileMap>();

    if (ImGui::CollapsingHeader("Tile Map")) {
        ImGui::Checkbox("Enable", &m_gizmoState.showGrid);

        ImGui::InputInt("Grid Columns", &tm->columns);
        ImGui::InputInt("Grid rows", &tm->rows);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputFloat("Tile Width", &tm->tileW, 1.f);
        ImGui::InputFloat("Tile Height", &tm->tileH, 1.f);


        if (ImGui::Button("Remove TileMap", ImVec2(-1, 0))) {
            // add a delete component func here
            selected->removeComponent<TileMap>();
            DebugLog::addMessage("Removed Tile Map Component");
        }
    }
}

void InspectorWindow::renderLua(GameObject* selected) {
	LuaScript* script = selected->getComponent<LuaScript>();

    if (ImGui::CollapsingHeader("Lua Script")) {

        /* ---------- reload script list button ---------- */
        if (ImGui::Button("Refresh Script List") || Editor::scriptState.availScripts.empty()) {
            Editor::scriptState.availScripts.clear();
			
            if (std::filesystem::exists(Editor::scriptState.scriptDir)) {
				for (const auto& entry : std::filesystem::directory_iterator(Editor::scriptState.scriptDir)) {
					if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                        Editor::scriptState.availScripts.push_back(entry.path().filename().string());
					}
				}
            }
        }
        /* ---------- END ---------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ---------- Dropdown to Select Script ---------- */
		int scriptIndex = -1;
        for (int i = 0; i < script->scripts.size(); ++i) {
            ImGui::PushID(i);
			LuaScript::ScriptEntry& scriptEntry = script->scripts[i];

			ImGui::Text("Script %d:", i + 1);
			std::string displayName = scriptEntry.scriptName.empty() ? "None" : scriptEntry.scriptName;

            if (ImGui::BeginCombo("##ScriptSelector", displayName.c_str())) {
                // clear script
                if (ImGui::Selectable("None", scriptEntry.scriptName.empty())) {
                    scriptEntry.scriptName = "";
                    scriptEntry.filename = "";
                    scriptEntry.changed = true;

                    DebugLog::addMessage("Cleared script " + displayName + " for " + selected->getObjectName());
                }

                // list all scripts
                for (const std::string& scriptFile : Editor::scriptState.availScripts) {
                    // check if this script is selected (compare with scriptName)
                    std::string nameWithoutExt = scriptFile.substr(0, scriptFile.find_last_of('.'));
                    bool isSelected = (scriptEntry.scriptName == nameWithoutExt);

                    if (ImGui::Selectable(scriptFile.c_str(), isSelected)) {
                        scriptEntry.scriptName = nameWithoutExt;
                        scriptEntry.filename = Editor::scriptState.scriptDir + scriptFile;
                        scriptEntry.changed = true;
                        script->scriptsChanged = true;

                        DebugLog::addMessage("Assigned script " + scriptFile + " to slot " + std::to_string(i + 1) + "\n");
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (!scriptEntry.filename.empty()) {
                ImGui::SameLine();
                if (ImGui::Button("Edit")) {
                    std::string command;
                #ifdef _WIN32
                    command = "code \"" + scriptEntry.filename + "\"";
                #elif __APPLE__
                    command = "open -a 'Visual Studio Code' \"" + scriptEntry.filename + "\"";
                #else
                   command = "code \"" + scriptEntry.filename + "\"";
                #endif
                    system(command.c_str());
                    DebugLog::addMessage("Opening script: " + scriptEntry.filename + "\n");
                }

                // tooltip on hover
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Open in VS Code");
                }
            }

            // remove this script
            ImGui::SameLine();
            if (script->scripts.size() > 1) {
                if (ImGui::Button("X")) {
                    scriptIndex = i;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Remove script");
                }
            }

            ImGui::Spacing();
            ImGui::PopID();
        }

        // Remove script if requested (do this after the loop)
        if (scriptIndex >= 0) {
            script->scripts.erase(script->scripts.begin() + scriptIndex);
            script->scriptsChanged = true;
            DebugLog::addMessage("Removed script slot " + std::to_string(scriptIndex + 1) + "\n");
        }

        /* ---------- Add new script button ---------- */
        if (ImGui::Button("+ Add Script", ImVec2(-1, 0))) {
            script->scripts.push_back(LuaScript::ScriptEntry());
            DebugLog::addMessage("Added new script slot\n");
        }

        /* ---------- END ---------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

   
        // Remove component button
        if (ImGui::Button("Remove Lua Script", ImVec2(-1, 0))) {
            selected->removeComponent<LuaScript>();
            DebugLog::addMessage("Removed Lua Script Component");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void InspectorWindow::renderVisualLogic(GameObject* selected) {
    VisualLC* logic = selected->getComponent<VisualLC>();

	if (ImGui::CollapsingHeader("Visual Logic")) {
        std::string displayName = logic->filename.empty() ? "<Create New>" : logic->filename;


		/* ---------- Dropdown to Select Logic Graph ---------- */
        if (ImGui::BeginCombo("Logic Graph", displayName.c_str())) {
            // create new logic asset
			if (ImGui::Selectable("<Create New>", logic->filename.empty())) {
                Editor::visualLogicState.showEditor = true;
                Editor::visualLogicState.currentGraphName = "";
				Editor::visualLogicState.targetObj = selected;

                // create new empty graph
                Editor::visualLogicState.nodes.clear();
                Editor::visualLogicState.connections.clear();
                logic->filename = "";

                VisualLogicEditor::Instance().updateNextNodeId();

				DebugLog::addMessage("Creating new Visual Logic Graph for " + selected->getObjectName());
			}

            // list existing logic assets
            if (std::filesystem::exists("assets/Logic")) {
                for (const auto& entry : std::filesystem::directory_iterator("assets/Logic")) {
                    if (entry.is_regular_file() && entry.path().extension() != ".json") {
                        continue;
                    }

                    std::string assetName = entry.path().filename().string();
                    bool isSelected = (logic->filename == assetName);

                    if (ImGui::Selectable(assetName.c_str(), isSelected)) {
                        logic->filename = assetName;

                        std::string filepath = "/Logic/" + assetName;
                        std::string err;
                        if (JsonIO::loadVisualLogic(filepath, logic->nodes, logic->connections, &err)) {
                            DebugLog::addMessage("Loaded Visual Logic Graph: " + assetName + "\n");
                            DebugLog::addMessage("Loaded " + std::to_string(logic->nodes.size()) + " nodes\n");
                        }
                        else {
                            DebugLog::addMessage("ERROR loading Visual Logic: " + err + "\n");
                        }
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }

			ImGui::EndCombo();
        }
        /* ---------- END ---------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ---------- Edit Logic Graph ---------- */
        if (!logic->filename.empty() && ImGui::Button("Edit Logic Graph", ImVec2(-1, 0))) {
            Editor::visualLogicState.showEditor = true;
			Editor::visualLogicState.currentGraphName = logic->filename;
            Editor::visualLogicState.targetObj = selected;
            
            // loading logic
            if (!logic->filename.empty()) {

                std::string filepath = "/Logic/" + logic->filename;
                std::string err;
                if (JsonIO::loadVisualLogic(filepath, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {
                    VisualLogicEditor::Instance().updateNextNodeId();

                    DebugLog::addMessage("Loaded Visual Logic: " + logic->filename + "\n");
                }
                else {
                    DebugLog::addMessage("ERROR loading Visual Logic: " + err + "\n");
                    // if file doesn't exist, try loading from component
                    Editor::visualLogicState.nodes = logic->nodes;
                    Editor::visualLogicState.connections = logic->connections;
                }
            }
            else {
                // no filename, load from component (new logic)
                Editor::visualLogicState.nodes = logic->nodes;
                Editor::visualLogicState.connections = logic->connections;
            }

            DebugLog::addMessage("Opening Visual Logic Editor for " + selected->getObjectName() + "\n");
        }
        /* ---------- END ---------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


		if (ImGui::Button("Remove Virtual Logic", ImVec2(-1, 0))) {
			selected->removeComponent<VisualLC>();
			DebugLog::addMessage("Removed Virtual Logic Controller");
		}
	}
}

void InspectorWindow::wrongFileTypePopup(const std::string& filename) {
    // open popup if flagged
    if (Editor::popupState.textureFileTypePopup) {
        ImGui::OpenPopup("Unsupported File Type");
        Editor::popupState.textureFileTypePopup = false;

        Editor::popupState.message = "Unsupported file type: " + filename + "\nTexture must be .png or .jpg files.";
    }
    else if (Editor::popupState.audioFileTypePopup) {
        ImGui::OpenPopup("Unsupported File Type");
        Editor::popupState.audioFileTypePopup = false;

        Editor::popupState.message = "Unsupported file type: " + filename + "\nAudio must be .wav, .ogg or .mp3 files.";
    }
    else if (Editor::popupState.prefabFileTypePopup) {
        ImGui::OpenPopup("Unsupported File Type");
        Editor::popupState.prefabFileTypePopup = false;

        Editor::popupState.message = "Unsupported file type: " + filename + "\nPlease drop a prefab file.";
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // wrong file type popup
    if (ImGui::BeginPopupModal("Unsupported File Type", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(Editor::popupState.message.c_str());

        if (ImGui::Button("OK") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            Editor::popupState.message.clear();
            Editor::popupState.filename.clear();

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

#endif