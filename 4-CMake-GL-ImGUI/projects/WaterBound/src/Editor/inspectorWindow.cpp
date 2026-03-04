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
#include "Systems.h"

InspectorWindow::InspectorWindow(GameObjectManager& manager, Editor::ObjSelectionState& Ostate, Editor::GizmoState& gState) :
    m_manager(manager),
    m_objSelectionState(Ostate),
    m_gizmoState(gState) {
}

void InspectorWindow::render() {
    ImGui::Begin("Inspector");

    GameObject* selected = nullptr;
    if (m_objSelectionState.selectedObject) selected = m_objSelectionState.selectedObject;
    else if (m_objSelectionState.selectedPrefab) {
        selected = m_objSelectionState.selectedPrefab.get();

        if (ImGui::Button("Save")) {
            const std::string& prefabID = selected->getObjectPrefabID();

            if (PrefabManager::Instance().savePrefab(selected)) {
                int count = PrefabManager::Instance().applyToAllInstances(prefabID, m_manager);
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
                if (m_manager.renameGameObject(m_objSelectionState.selectedObject, newObjName)) {
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

            renderTransform(selected);
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
            renderCollision(selected/*, manager*/);

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

        /* ------------------- TILE MAP ------------------- */
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

        /* ------------------- Particle System ------------------- */
        if (selected->hasComponent<Particle>()) {
            renderParticles(selected);

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

        /* ------------------- Visual LC ------------------- */
        if (selected->hasComponent<WaterBody>()) {
            renderWaterBody(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Visual LC ------------------- */
        if (selected->hasComponent<WeaponComponent>()) {
            renderWeapon(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Visual LC ------------------- */
        if (selected->hasComponent<Button>()) {
            renderButton(selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */

        /* ------------------- Camera ------------------- */
        if(selected->hasComponent<camera>()) {
            renderCamera(selected);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
		}
		/* ------------------- END ------------------- */

        /* ------------------- Layer Controls ------------------- */
        renderLayerControls(selected);
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

    ImGui::BeginChild("AddComponentSection", ImVec2(0, 400), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("Add Component");
    ImGui::Separator();

    static char search[64] = "";
    ImGui::InputTextWithHint("##search", "Search components...", search, sizeof(search));

    for (const Editor::ComponentEntry& comp : Editor::componentRegistry) {
        // skip if selected obj alr has this component
        if (comp.hasComp(selected)) continue;
        if (comp.name == "Animation") continue;

        // only one camera component may exist in one scene
        if (Editor::hasCam && comp.name == "Camera") continue;

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

            // record for undo/redo
            UndoRedoManager::Instance().executeCmd(std::make_unique<AddComponentCmd>(m_manager, selected, comp));

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

void InspectorWindow::renderTransform(GameObject* selected) {
    //Transform* transform, CollisionInfo* collision
    Transform* transform = selected->getComponent<Transform>();
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();

    if (ImGui::CollapsingHeader("Transform")) {

        if (ImGui::TreeNode("Position")) {
            bool posActive = false;

            float newX = transform->x;
            if (ImGui::DragFloat("X", &newX, 0.1f)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }
                TransformSystem::setWorldPosition(transform, newX, transform->y, m_manager);
            }
            posActive |= ImGui::IsItemActive();

            float newY = transform->y;
            if (ImGui::DragFloat("Y", &newY, 0.1f)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }
                TransformSystem::setWorldPosition(transform, transform->x, newY, m_manager);
            }
            posActive |= ImGui::IsItemActive();

            // set z to stay within the editor camera (-1.f <= z < 1.f)
            glm::mat4& proj = renderer::editorCam.proj;
            float m22 = proj[2][2];
            float m32 = proj[3][2];
            float near1 = (m32 - 1.f) / m22;
            float far1 = (m32 + 1.f) / m22;

            float newZ = transform->z;
            if (ImGui::DragFloat("Z", &newZ, 0.1f, far1, near1)) {
                if (!m_isEditingPos) {
                    UndoRedoManager::Instance().beginTransformEdit(selected);
                    m_isEditingPos = true;
                    m_transformEditObject = selected;
                }

                transform->z = std::clamp(newZ, far1, near1);
                //transform->z = newZ;
            }
            posActive |= ImGui::IsItemActive();

            if (m_isEditingPos && !posActive) {
                UndoRedoManager::Instance().endTransformEdit(m_manager, m_transformEditObject);
                m_isEditingPos = false;
            }

            ImGui::TreePop();
        }

        /* ------------------- Object Rotation ------------------- */
        if (ImGui::TreeNode("Rotation")) {
            float newRot = transform->rotation;
            if (ImGui::DragFloat("Angle", &newRot)) {
                TransformSystem::setWorldRotation(transform, newRot, m_manager);
            }

            // take snapshot when FIRST activated (before any changes)
            if (ImGui::IsItemActivated()) {
                UndoRedoManager::Instance().beginTransformEdit(selected);
                m_isEditingRot = true;
                m_transformEditObject = selected;
            }

            // end snapshot when deactivated (after all changes)
            if (ImGui::IsItemDeactivated() && m_isEditingRot) {
                UndoRedoManager::Instance().endTransformEdit(m_manager, m_transformEditObject);
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
                UndoRedoManager::Instance().endTransformEdit(m_manager, m_transformEditObject);
                m_isEditingScale = false;
            }

            ImGui::TreePop();

        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::TreeNode("Hierarchy")) {
            Transform* t = selected->getComponent<Transform>();

            // Assign parent via drag-drop from hierarchy window
            std::string currentParent = t->parentName.empty() ? "(none)" : t->parentName;
            ImGui::Text("Parent: %s", currentParent.c_str());
            ImGui::TextDisabled("(set parent by dragging in Hierarchy)");
            

            // Local offset display (read-only — managed by TransformSystem)
            if (!t->parentName.empty()) {
                ImGui::Spacing();
                ImGui::Text("Local Offset: (%.2f, %.2f)", t->localOffset.x, t->localOffset.y);
                ImGui::Text("Local Rotation: %.2f deg", t->localRotation);

                ImGui::Spacing();
                if (ImGui::Button("Unparent", ImVec2(-1, 0))) {
                    auto cmd = std::make_unique<ReparentCmd>(m_manager, selected->getObjectName(),
                        t->parentName, t->localOffset, t->localRotation,  // old state
                        "", Vector2D{ 0.f, 0.f }, 0.f);                     // new state (no parent)
                    UndoRedoManager::Instance().executeCmd(std::move(cmd));

                    t->parentName.clear();
                    t->localOffset = { 0.f, 0.f };
                    t->localRotation = 0.f;
                    CachedGameObjects::Instance().refresh();
                    DebugLog::addMessage(selected->getObjectName() + " unparented");
                }
            }

            ImGui::TreePop();
        }
    }
}

void InspectorWindow::renderRender(GameObject* selected) {
    Render* render = selected->getComponent<Render>();
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();

    // cache render component entry
    const Editor::ComponentEntry* entry = Editor::findEntry("Render");


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
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        render->texFile = path;
                        render->hasTex = true;
                        render->texChanged = true;

                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

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

                ImGui::SliderFloat("Alpha", &render->clr.w, 0.f, 1.f);
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                ImGui::Spacing();

                if (ImGui::Button("Clear Texture")) {
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    //glDeleteTextures(1, &render->texHDL);
                    render->texHDL = 0;
                    render->hasTex = false;
                    render->texFile.clear();
                    render->texChanged = true;

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Shape")) {
            const char* shapes[] = { "Rectangle", "Circle" };
            int current = (render->modelRef.shape == shape::square) ? 0 : 1;

            if (ImGui::Combo("##Shape", &current, shapes, IM_ARRAYSIZE(shapes))) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                if (current == 0) render->modelRef.shape = shape::square;
                else render->modelRef.shape = shape::circle;

                if (collision && collision->autoFitScale) collision->colliderType = render->modelRef.shape;

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Colour")) {

            ImGui::ColorEdit4("##Colour", &render->clr.x); // pass pointer of the first float cuz imgui want that

            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            static glm::vec4 prevColour = render->clr;
            static bool colourEditing = false;

            if (!colourEditing && render->clr != prevColour) {
                colourEditing = true;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            }
            if (colourEditing && ImGui::IsMouseReleased(0)) {
                colourEditing = false;
                prevColour = render->clr;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::TreePop();
        }


        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Render", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry)
                );

                DebugLog::addMessage("Removed Render Component");
            }
        }
    }
}

void InspectorWindow::renderCamera(GameObject* selected) {
    camera* cam = selected->getComponent<camera>();
    const Editor::ComponentEntry* entry = Editor::findEntry("Camera");

    if (ImGui::CollapsingHeader("Camera")) {

        if (ImGui::TreeNode("Viewport")) {
            // Toggle between screen size and custom size
            if (ImGui::Checkbox("Use Screen Size", &cam->useScreenSize)) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            // Only allow editing width/height if not using screen size
            ImGui::BeginDisabled(cam->useScreenSize);

            if (ImGui::DragFloat("Height", &cam->height, 0.1f, 0.1f, 10000.f)) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            if (ImGui::DragInt("Width", &cam->width, 1.f, 1, 10000)) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::EndDisabled();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Zoom")) {
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Zoom", &cam->zoom, 0.01f, cam->minZoom, cam->maxZoom);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Min Zoom", &cam->minZoom, 0.01f, 0.f, cam->maxZoom);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Max Zoom", &cam->maxZoom, 0.01f, cam->minZoom, 10000.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Map Bounds")) {
            ImGui::DragFloat("Map Width", &cam->mapWidth, 0.1f, 0.f, 100000.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Map Height", &cam->mapHeight, 0.1f, 0.f, 100000.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Camera", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry)
                );
                DebugLog::addMessage("Removed Camera Component");
            }
        }
    }
}

void InspectorWindow::renderStateMachine(GameObject* selected) {
    Render* render = selected->getComponent<Render>();
    Animation* animation = selected->getComponent<Animation>();

    // cache state machine/animation component entry
    const Editor::ComponentEntry* smEntry = Editor::findEntry("State Machine");
    const Editor::ComponentEntry* entry = Editor::findEntry("Animation");

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
                    if (entry) UndoRedoManager::Instance().executeCmd(
                        std::make_unique<AddComponentCmd>(m_manager, selected, *entry)
                    );
                    else selected->addComponent<Animation>();
                }
            }
            else if (!animation->animState.empty()) {
                if (ImGui::Checkbox("Is animated", &render->hasAnimation)) {
                    const Editor::ComponentEntry* renderEntry = Editor::findEntry("Render");

                    render->hasAnimation = !render->hasAnimation;
                    if (renderEntry) UndoRedoManager::Instance().beginComponentEdit(selected, *renderEntry);

                    render->hasAnimation = !render->hasAnimation;
                    if (renderEntry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *renderEntry);
                }

                ImGui::Separator();

                ImGui::BeginDisabled(!render->hasAnimation);

                static int selectedStateIndex = 0;
                const std::vector<std::string>& stateNames = JsonIO::stateNames;

                // Dropdown to select state
                if (ImGui::BeginCombo("State", stateNames[selectedStateIndex].c_str())) {
                    for (int i = 0; i < stateNames.size(); ++i) {
                        bool isSelected = (selectedStateIndex == i);
                        if (ImGui::Selectable(stateNames[i].c_str(), isSelected)) selectedStateIndex = i;

                        if (isSelected) ImGui::SetItemDefaultFocus();
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
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                                currentState.texFile = path;
                                currentState.texChanged = true;

                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

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

                    if (!currentState.texFile.empty()) {
                        if (ImGui::Button("Clear Texture")) {
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            //glDeleteTextures(1, &currentState.texHDL);
                            currentState.texHDL = 0;
                            currentState.texFile.clear();
                            currentState.texChanged = true;

                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // play animation
                    if (ImGui::Checkbox("Play Animation", &currentState.play)) {
                        currentState.play = !currentState.play;
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        currentState.play = !currentState.play;
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                    }

                    // loop animation
                    if (ImGui::Checkbox("Loop", &currentState.loop)) {
                        currentState.loop = !currentState.loop;
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        currentState.loop = !currentState.loop;
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                    }

                    // total column
                    if (ImGui::DragInt("Total Columns", &currentState.totalColumn))
                        currentState.totalColumn = std::max(currentState.totalColumn, 1);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit()) 
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    // total rows
                    if (ImGui::DragInt("Total Rows", &currentState.totalRow))
                        currentState.totalRow = std::max(currentState.totalRow, 1);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::DragFloat("Initial Column", &currentState.initialFrame.x, 1.f, 0.f, 100.f, "%.0f");
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::DragFloat("Initial Row", &currentState.initialFrame.y, 1.f, 0.f, 100.f, "%.0f");
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::DragFloat("Last Column", &currentState.lastFrame.x, 1.f, 0.f, 100.f, "%.0f");
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::DragFloat("Last Row", &currentState.lastFrame.y, 1.f, 0.f, 100.f, "%.0f");
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    // frame time
                    if (ImGui::DragFloat("Frame Time", &currentState.frameTime, 0.001f))
                        currentState.frameTime = std::max(currentState.frameTime, 0.001f);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::EndDisabled();
                }

                ImGui::EndDisabled();

                ImGui::Separator();
                if (entry) {
                    if (ImGui::Button("Remove Animation", ImVec2(-1, 0))) {
                        UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                        DebugLog::addMessage("Removed Animation Component\n");
                    }
                }

            }
            else {

                if (entry) {
                    if (ImGui::Button("Remove Animation", ImVec2(-1, 0))) {
                        UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                        DebugLog::addMessage("Removed Animation Component\n");
                    }
                }
            }

            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (smEntry) {
            if (ImGui::Button("Remove State Machine", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *smEntry));

                DebugLog::addMessage("Removed State Machine\n");
            }
        }

    }
}

void InspectorWindow::renderCollision(GameObject* selected/*, GameObjectManager& manager*/) {
    CollisionInfo* collision = selected->getComponent<CollisionInfo>();
    Transform* transform = selected->getComponent<Transform>();
    Render* render = selected->getComponent<Render>();

    // cache collision component entry
    const Editor::ComponentEntry* entry = Editor::findEntry("Collision");

    if (ImGui::CollapsingHeader("Collision")) {
        {
            // checked means this object can collide with obj in the same layer
            if (ImGui::Checkbox("Enable", &collision->collisionFlag)) {
                collision->collisionFlag = !collision->collisionFlag;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                collision->collisionFlag = !collision->collisionFlag;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::TreeNode("Collider")) {
            if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = true;

            // cannot edit if obj cannot collide
            ImGui::BeginDisabled(!collision->collisionFlag);
            if (ImGui::Checkbox("Auto-fit Scale", &collision->autoFitScale)) {
                collision->autoFitScale = !collision->autoFitScale;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                collision->autoFitScale = !collision->autoFitScale;

                if (collision->autoFitScale && transform) {
                    collision->colliderSize.x = transform->scaleX;
                    collision->colliderSize.y = transform->scaleY;
                }

                if (collision->autoFitScale && render) {
                    collision->colliderType = render->modelRef.shape;
                }

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            CollisionResponseMode mode = collision->collisionRes;
            const char* modes[] = { "Static", "Pushable", "Kinematic" };
            int current = static_cast<int>(mode);

            if (ImGui::Combo("Collision Response", &current, modes, IM_ARRAYSIZE(modes))) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                collision->collisionRes = static_cast<CollisionResponseMode>(current);

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::EndDisabled();

            // cannot edit if obj cannot collide or auto-fit collider size with obj size is on
            bool editable = collision->collisionFlag && !collision->autoFitScale;
            ImGui::BeginDisabled(!editable);

            shape colliderShape = collision->colliderType;
            const char* shapes[] = { "Box", "Circle" };
            current = static_cast<int>(colliderShape);

            if (ImGui::Combo("Shape", &current, shapes, IM_ARRAYSIZE(shapes))) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                collision->colliderType = static_cast<shape>(current);
                if (collision->colliderType == shape::circle)
                    collision->colliderRot = 0.f;

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            if (collision->colliderType == shape::circle) {
                // diameter
                if (ImGui::DragFloat("Diamater", &collision->colliderSize.x, 0.1f, 0.f, FLT_MAX)) {
                    collision->colliderSize.x = std::max(collision->colliderSize.x, 0.f);
                    collision->colliderSize.y = collision->colliderSize.x;
                    if (Physics* p = selected->getComponent<Physics>()) {
                        if (!p->freezeRotation) p->recalculateInertia(collision->colliderSize, shape::circle);
                    }
                }
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                // position offset
                if (ImGui::DragFloat2("Offset", &collision->colliderPos.x, 0.1f)) {}
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }
            else {
                if (ImGui::DragFloat("Width", &collision->colliderSize.x, 0.1f, 0.f, FLT_MAX)){
                    collision->colliderSize.x = std::max(collision->colliderSize.x, 0.f);

                    if (Physics* p = selected->getComponent<Physics>()) {
                        if (!p->freezeRotation) p->recalculateInertia(collision->colliderSize, collision->colliderType);
                    }
                }
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);;

                if (ImGui::DragFloat("Height", &collision->colliderSize.y, 0.1f, 0.f, FLT_MAX)){
                    collision->colliderSize.y = std::max(collision->colliderSize.y, 0.f);
                    
                    if (Physics* p = selected->getComponent<Physics>()) {
                        if (!p->freezeRotation) p->recalculateInertia(collision->colliderSize, collision->colliderType);
                    }
                }
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                // position offset
                if (ImGui::DragFloat2("Offset", &collision->colliderPos.x, 0.1f)) {}
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                // rotation
                if (ImGui::DragFloat("Rotation", &collision->colliderRot, 0.5f, 0.f, 360.f)) {
                    collision->colliderRot = std::clamp(collision->colliderRot, 0.f, 360.f);
                }
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);;
            }

            ImGui::EndDisabled();

            ImGui::TreePop();
        }
        else {
            if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = false;
        }

        ImGui::Spacing();

        if (ImGui::TreeNode("Physics Material")) {
            ImGui::BeginDisabled(!collision->collisionFlag);

            ImGui::DragFloat("Bounciness", &collision->material.bounciness, 0.1f, 0.0f, 1.0f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Friction", &collision->material.friction, 0.1f, 0.0f, 1.0f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            // Combine mode for bounciness
            const char* combineModes[] = { "Average", "Minimum", "Maximum", "Multiply" };
            int bounceMode = static_cast<int>(collision->material.bounceCombine);
            if (ImGui::Combo("Bounce Combine", &bounceMode, combineModes, IM_ARRAYSIZE(combineModes))) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                collision->material.bounceCombine = static_cast<PhysicsMaterial::CombineMode>(bounceMode);
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            int frictionMode = static_cast<int>(collision->material.frictionCombine);
            if (ImGui::Combo("Friction Combine", &frictionMode, combineModes, IM_ARRAYSIZE(combineModes))) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                collision->material.frictionCombine = static_cast<PhysicsMaterial::CombineMode>(frictionMode);
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::EndDisabled();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Collision Tag")) {
            std::string currentTag = collision->collisionTag.empty() ? "default" : collision->collisionTag;
            const std::vector<std::string>& collisionTags = GlobalRegistry::Instance().getCollisionTags();

            ImGui::BeginDisabled(!collision->collisionFlag);
            if (ImGui::BeginCombo("Object Tag", currentTag.c_str())) {
                for (const std::string& tag : collisionTags) {
                    bool isSelected = (currentTag == tag);

                    if (ImGui::Selectable(tag.c_str(), isSelected)) {
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        collision->collisionTag = tag;

                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                    }

                    if (isSelected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::Spacing();
            ImGui::Separator();

            // Ignore Collisions with Checkboxes
            ImGui::Text("Ignore Collisions With:");
            ImGui::Spacing();

            for (const std::string& tag : collisionTags) {
                bool isIgnored = (collision->ignoreCollisionsWith.find(tag) != collision->ignoreCollisionsWith.end());

                if (ImGui::Checkbox(tag.c_str(), &isIgnored)) {
                    isIgnored = !isIgnored;
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    isIgnored = !isIgnored;

                    if (isIgnored) {
                        collision->ignoreCollisionsWith.insert(tag);
                    }
                    else {
                        collision->ignoreCollisionsWith.erase(tag);
                    }

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                }
            }

            ImGui::Spacing();

            if (!collision->ignoreCollisionsWith.empty()) {
                if (ImGui::Button("Uncheck All", ImVec2(-1, 0))) {
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    collision->ignoreCollisionsWith.clear();

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                }
            }

            if (!collisionTags.empty() && collision->ignoreCollisionsWith.size() != collisionTags.size()) {
                if (ImGui::Button("Check All", ImVec2(-1, 0))) {
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    for (const std::string& tag : collisionTags) 
                        collision->ignoreCollisionsWith.insert(tag);

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                }
            }
            else if (collisionTags.empty()) {
                ImGui::TextWrapped("No collision tags available. Add collision tags in the Global Registry.");
            }

            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        if (entry) {
            if (ImGui::Button("Remove Collision", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Collision\n");
            }
        }
    }
    else {
        if (collision->collisionFlag) Editor::sceneWindowState.showSingleCollider = false;
    }
}

void InspectorWindow::renderInput(GameObject* selected) {
    if (ImGui::CollapsingHeader("Input Control")) {
        ImGui::TextWrapped("This object can be controlled by keyboard input");

        if (const Editor::ComponentEntry* entry = Editor::findEntry("Input Control")) {
            if (ImGui::Button("Remove Input Control", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry)
                );
                DebugLog::addMessage("Removed Input Control\n");
            }
        }
    }
}

void InspectorWindow::renderPhysics(GameObject* selected, bool canMove) {
    Physics* physic = selected->getComponent<Physics>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Physics");


    if (ImGui::CollapsingHeader("Physics")) {
        if (ImGui::Checkbox("Is Trigger", &physic->physicsFlag)) {
            physic->physicsFlag = !physic->physicsFlag;
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            physic->physicsFlag = !physic->physicsFlag;
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // --- flat fields ---
        // if (ImGui::InputFloat("Mass", &physic->dynamics.mass, 1.f)) {
        //     physic->dynamics.mass = std::max(physic->dynamics.mass, 0.f);
        //     if (!physic->freezeRotation) {
        //         if (auto* c = selected->getComponent<CollisionInfo>())
        //             physic->recalculateInertia(c->colliderSize, c->colliderType);

        // only show movement data if object is controlable via input
        if (ImGui::TreeNode("Properties")) {
            if (ImGui::DragFloat("Mass", &physic->dynamics.mass, 1.f, 0.f, FLT_MAX)) {
                physic->dynamics.mass = std::max(physic->dynamics.mass, 0.f); // 0.f mass means static obj (never move, e.g. wall)

                if (!physic->freezeRotation) {
                    if (auto* c = selected->getComponent<CollisionInfo>())
                        physic->recalculateInertia(c->colliderSize, c->colliderType);
                }
            }
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            //if (ImGui::DragFloat("Gravity Scale", &physic->gravity, 1.f))
            //    physic->gravity = std::min(physic->gravity, 0.f); // gravity should stay negative

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Input Movement Control")) {
            if (canMove) {

                if (ImGui::TreeNode("Movement")) {
                    if (ImGui::DragFloat("Move Speed", &physic->moveSpeed, 1.f))
                        physic->moveSpeed = std::max(physic->moveSpeed, 0.f); // set the min speed to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    if (ImGui::DragFloat("Jump Force", &physic->jumpForce, 1.f))
                        physic->jumpForce = std::max(physic->jumpForce, 0.f); // set the min jump force to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    if (ImGui::DragFloat("Ground Accel", &physic->groundAccelForce, 1.0f))
                        physic->groundAccelForce = std::max(physic->groundAccelForce, 0.f); // set the min to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    if (ImGui::DragFloat("Air Accel", &physic->airAccelForce, 1.0f))
                        physic->airAccelForce = std::max(physic->airAccelForce, 0.f); // set the min to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    if (ImGui::DragFloat("Apex Hang Multiplier", &physic->apexHangMul, 0.1f))
                        physic->apexHangMul = std::clamp(physic->apexHangMul, 0.f, 1.f); // set the min to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    if (ImGui::DragFloat("Max Fall Speed", &physic->maxFallSpeed, 1.0f))
                        physic->maxFallSpeed = std::max(physic->maxFallSpeed, 0.f); // set the min to 0.f
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);


                    ImGui::TreePop();

                }

                if (ImGui::TreeNode("Jump Buffer")) {
                    ImGui::SliderFloat("Buffer Time", &physic->jumpBufferTime, 0.0f, 10.f);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Coyote Jump")) {
                    ImGui::SliderFloat("Coyote Time", &physic->coyoteTime, 0.0f, 10.f);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Ledge Assist")) {
                    ImGui::DragFloat("Assist Range", &physic->ledgeAssistRange, 0.1f);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::DragFloat("Climb Speed", &physic->ledgeClimbSpeed, 1.0f);
                    if (ImGui::IsItemActivated())
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                    if (ImGui::IsItemDeactivatedAfterEdit())
                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }


        // --- Constraints ---
        if (ImGui::TreeNode("Constraints")) {
            ImGui::Text("Freeze Rotation");
            ImGui::SameLine();

            if (ImGui::Checkbox("Z", &physic->freezeRotation)) {
                physic->freezeRotation = !physic->freezeRotation;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                physic->freezeRotation = !physic->freezeRotation;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            if (!physic->freezeRotation) {
                ImGui::Spacing();

                if (ImGui::DragFloat("Inertia Override", &physic->inertiaOverride, 0.1f)) {
                    physic->inertiaOverride = std::max(physic->inertiaOverride, 0.f);
                    if (auto* c = selected->getComponent<CollisionInfo>())
                        physic->recalculateInertia(c->colliderSize, c->colliderType);
                }
                if (ImGui::IsItemActivated())
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                if (ImGui::IsItemDeactivatedAfterEdit())
                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("0 = auto-calculate from mass and shape");

                ImGui::SameLine();
                if (ImGui::Button("Reset")) {
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    physic->inertiaOverride = 0.0f;
                    if (auto* c = selected->getComponent<CollisionInfo>())
                        physic->recalculateInertia(c->colliderSize, c->colliderType);

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Reset to auto-calculated value");
                ImGui::Text("Inertia: %.4f", physic->dynamics.inertia);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Environment")) {
            if (ImGui::DragFloat("Air Drag", &physic->damping, 0.1f))
                physic->damping = std::clamp(physic->damping, 0.f, 1.f); // damping within 0.1 to 1.f
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            if (ImGui::DragFloat("Angular Drag", &physic->dynamics.angularDrag, 0.1f, 0.f))
                physic->dynamics.angularDrag = std::clamp(physic->dynamics.angularDrag, 0.f, 1.0f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Water")) {
            if (ImGui::Checkbox("Buoyancy", &physic->buoyancy)) {
                physic->buoyancy = !physic->buoyancy;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                physic->buoyancy = !physic->buoyancy;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Physics Info")) {
            
            ImGui::Text("Velocity: (%.2f, %.2f)", physic->dynamics.velocity.x, physic->dynamics.velocity.y);
            Vector2D vel2D{ physic->dynamics.velocity.x, physic->dynamics.velocity.y };
            ImGui::Text("Speed: %.2f", Vec_Length(&vel2D));
            ImGui::Text("Angular Velocity: %.4f", physic->dynamics.angularVelocity);
            ImGui::Text("Inverse Mass: %.4f", physic->dynamics.inverseMass);
            ImGui::Text("Coyote Available: %s", physic->coyoteAvailable ? "true" : "false");
            ImGui::Text("Jump Buffer: %.3f", physic->jumpBufferTimer);
            ImGui::Text("Ledge Climbing: %s", physic->ledgeClimb ? "true" : "false");

            ImGui::TreePop();
        }

        if (entry) {
            if (ImGui::Button("Remove Physics", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Physics\n");
            }
        }
    }
}

void InspectorWindow::renderFont(GameObject* selected) {
    FontComponent* font = selected->getComponent<FontComponent>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Font");

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
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            // size of the text
            if (ImGui::DragFloat("Size", &font->scale, 1.f))
                font->scale = std::max(font->scale, 0.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            // colour of the text
            ImGui::ColorEdit3("Colour", &font->clr.x);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            
            static glm::vec3 prevColour = font->clr;
            static bool colourEditing = false;

            if (!colourEditing && font->clr != prevColour) {
                colourEditing = true;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            }
            if (colourEditing && ImGui::IsMouseReleased(0)) {
                colourEditing = false;
                prevColour = font->clr;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            // font type of the text
            /* scuffed way to select font */
            static const char* fontNames[] = {
                "Orange Knight",
                "Arial",
                "Times New Roman"
            };

            int prevFontType = font->fontType;
            if (ImGui::Combo("Font Type", &font->fontType, fontNames, IM_ARRAYSIZE(fontNames))) {
                if (entry) {
                    std::swap(font->fontType, prevFontType);
                    UndoRedoManager::Instance().beginComponentEdit(selected, *entry); // before 

                    std::swap(font->fontType, prevFontType);
                    UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry); // after
                }
            }
        }

        if (entry) {
            if (ImGui::Button("Remove Font", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Font\n");
            }
        }
    }
}

void InspectorWindow::renderAudio(GameObject* selected) {
    AudioComponent* audio = selected->getComponent<AudioComponent>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Audio");

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
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    audio->getOrCreateChannel(channelName);

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    DebugLog::addMessage("Added audio channel: " + channelName + "\n");
                    newChannelName[0] = '\0';
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
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                                ch.audioFile = path;

                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

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

                        if (ImGui::Checkbox("Loop", &ch.loop)) {
                            ch.loop = !ch.loop;
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            ch.loop = !ch.loop;
                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                            if (ch.channel) ch.isPendingStop = true;
                        }
                        if (ImGui::Checkbox("Play On Start", &ch.playOnStart)) {
                            ch.playOnStart = !ch.playOnStart;
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            ch.playOnStart = !ch.playOnStart;
                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                            if (ch.channel) ch.isPendingStop = true;
                        }
                        if (ImGui::Checkbox("Muted", &ch.muted)) {
                            ch.muted = !ch.muted;
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            ch.muted = !ch.muted;
                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                            if (ch.channel) ch.isPendingStop = true;
                        }
                        if (ImGui::Checkbox("Spatial Audio", &ch.isSpatial)) {
                            ch.isSpatial = !ch.isSpatial;
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            ch.isSpatial = !ch.isSpatial;
                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                        }

                        if (ch.isSpatial) {
                            ImGui::Indent();  // Indent to show hierarchy

                            if (ImGui::DragFloat("Min Distance", &ch.minDistance, 0.1f, 0.1f, 1000.0f)) {
                                ch.minDistance = std::clamp(ch.minDistance, 0.1f, 1000.0f);
                            }
                            if (ImGui::IsItemActivated())
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);


                            if (ImGui::DragFloat("Max Distance", &ch.maxDistance, 1.0f, 1.0f, 10000.0f)) {
                                ch.maxDistance = std::clamp(ch.maxDistance, 1.0f, 10000.0f);
                            }
                            if (ImGui::IsItemActivated())
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);


                            ImGui::Unindent();
                        }

                        ImGui::Spacing();

                        // Volume & Pitch Controls
                        if (ImGui::TreeNode("Volume & Pitch")) {
                            if (ImGui::DragFloat("Volume", &ch.volume, 0.01f, 0.0f, 1.0f)) {
                                ch.volume = std::clamp(ch.volume, 0.f, 1.f);
                            }
                            if (ImGui::IsItemActivated())
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);


                            if (ImGui::DragFloat("Pitch", &ch.pitch, 0.01f, 0.1f, 2.0f)) {
                                ch.pitch = std::clamp(ch.pitch, 0.1f, 2.f);
                            }
                            if (ImGui::IsItemActivated())
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                            if (ImGui::IsItemDeactivatedAfterEdit())
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);


                            ImGui::TreePop();
                        }

                        // Fade Controls
                        if (ImGui::TreeNode("Fade In/Out")) {
                            if (ImGui::Checkbox("Fade In On Start", &ch.fadeInOnStart)) {
                                ch.fadeInOnStart = !ch.fadeInOnStart;
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                                ch.fadeInOnStart = !ch.fadeInOnStart;
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                            }

                            if (ch.fadeInOnStart) {
                                ImGui::Indent();
                                if (ImGui::DragFloat("Fade In Duration", &ch.fadeInDuration, 0.01f, 0.0f, 10.0f))
                                    ch.fadeInDuration = std::clamp(ch.fadeInDuration, 0.f, 10.f);
                                if (ImGui::IsItemActivated())
                                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                                if (ImGui::IsItemDeactivatedAfterEdit())
                                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                                ImGui::Unindent();
                            }

                            if (ImGui::Checkbox("Fade Out On Stop", &ch.fadeOutOnStop)) {
                                ch.fadeOutOnStop = !ch.fadeOutOnStop;
                                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                                ch.fadeOutOnStop = !ch.fadeOutOnStop;
                                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                            }

                            if (ch.fadeOutOnStop) {
                                ImGui::Indent();
                                if (ImGui::DragFloat("Fade Out Duration", &ch.fadeOutDuration, 0.01f, 0.0f, 10.0f))
                                    ch.fadeOutDuration = std::clamp(ch.fadeOutDuration, 0.f, 10.f);
                                if (ImGui::IsItemActivated())
                                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                                if (ImGui::IsItemDeactivatedAfterEdit())
                                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                                ImGui::Unindent();
                            }

                            ImGui::TreePop();
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Remove Sound (clears file but keeps channel)
                        if (ImGui::Button("Clear Audio File", ImVec2(-1, 0))) {
                            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                            ch.audioFile = "";
                            if (ch.channel) {
                                ch.channel->stop();
                                ch.channel = nullptr;
                            }

                            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

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
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                audio->removeChannel(channelToRemove);

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
                DebugLog::addMessage("Removed audio channel: " + channelToRemove + "\n");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Audio", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Audio\n");
            }
        }
    }
}

void InspectorWindow::renderTileMap(GameObject* selected)
{
    TileMap* tm = selected->getComponent<TileMap>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Tile Map");

    if (ImGui::CollapsingHeader("Tile Map")) {
        if (ImGui::Checkbox("Enable Collision", &tm->enableCol)) {
            tm->enableCol = !tm->enableCol;
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            tm->enableCol = !tm->enableCol;
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
        }

        if (ImGui::TreeNode("Brush")) {
            std::string displayText = TileMapSystem::filename.empty() ? "No Brush Selected (Drag texture here)" : std::filesystem::path(TileMapSystem::filename).filename().string();

            ImGui::DragInt("Brush Size", &TileMapSystem::brushSize, 0.1f, 1, 5);
            if (ImGui::IsItemActivated())
                UndoRedoManager::Instance().beginTileMapToolEdit(m_gizmoState.showGrid);
            if (ImGui::IsItemDeactivatedAfterEdit())
                UndoRedoManager::Instance().endTileMapToolEdit(m_gizmoState.showGrid);

            ImGui::Text("(%dx%d tiles)", TileMapSystem::brushSize, TileMapSystem::brushSize);

            ImGui::BeginChild("BrushDropZone", ImVec2(-1, 80), true, ImGuiWindowFlags_NoScrollWithMouse);

            if (!TileMapSystem::filename.empty()) {
                TextureData texData = ResourceManager::getInstance().getTexture(TileMapSystem::filename);

                if (texData.id != 0) {
                    ImGui::Image((ImTextureID)(intptr_t)texData.id, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::SameLine();
                }
            }

            ImGui::TextWrapped("%s", displayText.c_str());

            // drag and drop from asset browser
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                    const char* droppedPath = (const char*)payload->Data;
                    std::string path = droppedPath;

                    std::string ext;
                    std::filesystem::path fsPath(path);
                    ext = fsPath.extension().string();

                    // only accept if dropped file is a png/jpg/jpeg
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        UndoRedoManager::Instance().beginTileMapToolEdit(m_gizmoState.showGrid);

                        TileMapSystem::filename = path;

                        UndoRedoManager::Instance().endTileMapToolEdit(m_gizmoState.showGrid);

                        DebugLog::addMessage("Selected brush texture: " + TileMapSystem::filename + "\n");
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

            ImGui::EndChild();

            

            // clear texture from brush
            if (!TileMapSystem::filename.empty()) {
                ImGui::Spacing();

                if (ImGui::Checkbox("Eraser Mode", &TileMapSystem::eraseMode)) {
                    TileMapSystem::eraseMode = !TileMapSystem::eraseMode;
                    UndoRedoManager::Instance().beginTileMapToolEdit(m_gizmoState.showGrid);
                    TileMapSystem::eraseMode = !TileMapSystem::eraseMode;
                    UndoRedoManager::Instance().endTileMapToolEdit(m_gizmoState.showGrid);
                }

                ImGui::Spacing();

                if (ImGui::Button("Clear Brush", ImVec2(-1, 0))) {
                    UndoRedoManager::Instance().beginTileMapToolEdit(m_gizmoState.showGrid);
                    TileMapSystem::filename.clear();
                    TileMapSystem::eraseMode = false;
                    UndoRedoManager::Instance().endTileMapToolEdit(m_gizmoState.showGrid);
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Grid")) {
            if (ImGui::Checkbox("Enable", &m_gizmoState.showGrid)) {
                m_gizmoState.showGrid = !m_gizmoState.showGrid;
                UndoRedoManager::Instance().beginTileMapToolEdit(m_gizmoState.showGrid);
                m_gizmoState.showGrid = !m_gizmoState.showGrid;
                UndoRedoManager::Instance().endTileMapToolEdit(m_gizmoState.showGrid);
            }

            ImGui::DragInt("Grid Columns", &tm->columns);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragInt("Grid rows", &tm->rows);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::DragFloat("Tile Width", &tm->tileW, 1.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Tile Height", &tm->tileH, 1.f);
            if (ImGui::IsItemActivated())
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit())
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        if (entry) {
            if (ImGui::Button("Remove Tile Map", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Tile Map\n");
            }
        }
    }
}

void InspectorWindow::renderParticles(GameObject* selected)
{
    Particle* ps = selected->getComponent<Particle>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Particle");

    if (ImGui::CollapsingHeader("Particle System")) {
        ImGui::DragInt("Max Particles", &ps->master.particleAmt);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::DragFloat("Particle Lifetime", &ps->master.timer, 0.1f);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::DragFloat("Offset", &ps->master.offset, 0.1f);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::DragFloat("Velocity X", &ps->master.velocity.x, 0.1f);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::DragFloat("Velocity Y", &ps->master.velocity.y, 0.1f);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::ColorEdit3("Start Colour", &ps->master.clr.x);
        if (ImGui::IsItemActivated())
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit())
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        static bool colourPickerWasOpen = false;
        bool colourPickerOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup);
        if (colourPickerOpen && !colourPickerWasOpen)
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (!colourPickerOpen && colourPickerWasOpen)
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
        colourPickerWasOpen = colourPickerOpen;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Particle", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));
                DebugLog::addMessage("Removed Particle\n");
            }
        }
    }
}

void InspectorWindow::renderLua(GameObject* selected) {
    LuaScript* script = selected->getComponent<LuaScript>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Lua Script");

    if (ImGui::CollapsingHeader("Lua Script")) {

        /* ---------- reload script list button ---------- */
        if (ImGui::Button("Refresh Script List") || Editor::scriptState.availScripts.empty()) {
            Editor::scriptState.availScripts.clear();

            if (std::filesystem::exists(Editor::scriptState.scriptDir)) {
                for (const auto& file : std::filesystem::recursive_directory_iterator(Editor::scriptState.scriptDir)) {
                    if (file.is_regular_file() && file.path().extension() == ".lua") {
                        // store relative path from scriptDir e.g. "subfolder/script.lua"
                        std::string relativePath = std::filesystem::relative(
                            file.path(), Editor::scriptState.scriptDir).string();
                        std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

                        Editor::scriptState.availScripts.push_back(relativePath);
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
                    if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                    scriptEntry.scriptName = "";
                    scriptEntry.filename = "";
                    scriptEntry.changed = true;

                    if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                    DebugLog::addMessage("Cleared script " + displayName + " for " + selected->getObjectName());
                }

                // list all scripts
                for (const std::string& scriptFile : Editor::scriptState.availScripts) {
                    // check if this script is selected (compare with scriptName)
                    std::filesystem::path scriptPath(scriptFile);
                    std::string nameWithoutExt = scriptPath.stem().string();

                    bool isSelected = (scriptEntry.scriptName == nameWithoutExt);

                    if (ImGui::Selectable(scriptFile.c_str(), isSelected)) {
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        scriptEntry.scriptName = nameWithoutExt;
                        scriptEntry.filename = Editor::scriptState.scriptDir + scriptFile;
                        scriptEntry.changed = true;
                        script->scriptsChanged = true;

                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

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
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

            script->scripts.erase(script->scripts.begin() + scriptIndex);
            script->scriptsChanged = true;

            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            DebugLog::addMessage("Removed script slot " + std::to_string(scriptIndex + 1) + "\n");
        }

        /* ---------- Add new script button ---------- */
        if (ImGui::Button("+ Add Script", ImVec2(-1, 0))) {
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

            script->scripts.push_back(LuaScript::ScriptEntry());
            script->scriptsChanged = true;

            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            DebugLog::addMessage("Added new script slot\n");
        }

        /* ---------- END ---------- */


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Lua Script", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Lua Script\n");
            }
        }
    }
}

void InspectorWindow::renderVisualLogic(GameObject* selected) {
    VisualLC* logic = selected->getComponent<VisualLC>();

    const Editor::ComponentEntry* entry = Editor::findEntry("Visual Logic");

    if (ImGui::CollapsingHeader("Visual Logic")) {
        std::string displayName = logic->filename.empty() ? "<Create New>" : logic->filename;


        /* ---------- Dropdown to Select Logic Graph ---------- */
        if (ImGui::BeginCombo("Logic Graph", displayName.c_str())) {
            // create new logic asset
            if (ImGui::Selectable("<Create New>", logic->filename.empty())) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                logic->filename.clear();
                logic->nodes.clear();
                logic->connections.clear();

                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

                VisualLogicEditor::Instance().clear();

                Editor::visualLogicState.showEditor = true;
                Editor::visualLogicState.targetObj = selected;

                DebugLog::addMessage("Creating new Visual Logic Graph for " + selected->getObjectName());
            }

            // list existing logic assets
            if (std::filesystem::exists("assets/Logic")) {
                for (const auto& file : std::filesystem::recursive_directory_iterator("assets/Logic")) {
                    if (!file.is_regular_file() && file.path().extension() != ".json") {
                        continue;
                    }

                    //std::string assetName = file.path().filename().string();
                    std::string relativePath = std::filesystem::relative(file.path(), "assets/Logic").string();

                    bool isSelected = (logic->filename == relativePath);

                    if (ImGui::Selectable(relativePath.c_str(), isSelected)) {
                        if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);

                        Editor::visualLogicState.targetObj = selected;
                        VisualLogicEditor::Instance().loadLogic(relativePath);

                        if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
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


        if (entry) {
            if (ImGui::Button("Remove Visual Logic", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));

                DebugLog::addMessage("Removed Visual Logic\n");
            }
        }
    }
}

void InspectorWindow::renderButton(GameObject* selected) {
    Button* b = selected->getComponent<Button>();
    const Editor::ComponentEntry* entry = Editor::findEntry("Button");
    if (ImGui::CollapsingHeader("Button")) {
        // action Name dropdown
        /*const char* actionNames[] = {
            "NONE", "START_GAME", "OPEN_CONTROL", "OPEN_AUDIO", "EXIT_GAME",
            "RESUME_GAME", "YES_EXIT", "NO_EXIT", "BACK", "MENU",
            "MUSIC_VOL_UP", "MUSIC_VOL_DOWN", "SFX_VOL_UP", "SFX_VOL_DOWN"
        };
        int currentAction = static_cast<int>(b->actionName);
        if (ImGui::Combo("Action", &currentAction, actionNames, IM_ARRAYSIZE(actionNames))) {
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            b->actionName = static_cast<ButtonAction>(currentAction);
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
        }*/

        // audio channel dropdowns
        AudioComponent* audio = selected->getComponent<AudioComponent>();
        if (audio && !audio->audioChannels.empty()) {
            // build channel list with "None" as first option
            std::vector<std::string> channelKeys;
            channelKeys.push_back("");
            for (auto& pair : audio->audioChannels)
                channelKeys.push_back(pair.first);

            std::vector<const char*> channelNames;
            channelNames.push_back("None");
            for (int i = 1; i < (int)channelKeys.size(); i++)
                channelNames.push_back(channelKeys[i].c_str());

            // On Click Channel
            int clickIdx = 0;
            for (int i = 0; i < (int)channelKeys.size(); i++)
                if (channelKeys[i] == b->onClickChannel) { clickIdx = i; break; }

            if (ImGui::Combo("On Click Channel", &clickIdx, channelNames.data(), (int)channelNames.size())) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                b->onClickChannel = channelKeys[clickIdx];
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            // On Hover Channel
            int hoverIdx = 0;
            for (int i = 0; i < (int)channelKeys.size(); i++)
                if (channelKeys[i] == b->onHoverChannel) { hoverIdx = i; break; }

            if (ImGui::Combo("On Hover Channel", &hoverIdx, channelNames.data(), (int)channelNames.size())) {
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                b->onHoverChannel = channelKeys[hoverIdx];
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }
        }
        else {
            ImGui::TextDisabled("No AudioComponent / channels found on this object");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Button", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));
                DebugLog::addMessage("Removed Button\n");
            }
        }
    }
}

void InspectorWindow::renderWaterBody(GameObject* selected) {
    WaterBody* w = selected->getComponent<WaterBody>();
    const Editor::ComponentEntry* entry = Editor::findEntry("Water Body");

    if (ImGui::CollapsingHeader("Water Body")) {

        if (ImGui::TreeNode("Waves")) {
            ImGui::DragFloat("Amplitude", &w->waveAmplitude, 0.01f, 0.f, 10.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Frequency", &w->waveFrequency, 0.01f, 0.f, 20.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Speed", &w->waveSpeed, 0.01f, 0.f, 20.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            bool renderChanged = ImGui::Checkbox("Render Waves", &w->renderWaves);
            if (renderChanged) {
                w->renderWaves = !w->renderWaves;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                w->renderWaves = !w->renderWaves;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Appearance")) {
            ImGui::ColorEdit4("Water Colour", &w->waterColor.r);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            static bool colourPickerWasOpen = false;
            bool colourPickerOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup);
            if (colourPickerOpen && !colourPickerWasOpen)
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (!colourPickerOpen && colourPickerWasOpen)
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            colourPickerWasOpen = colourPickerOpen;

            ImGui::DragFloat("Alpha", &w->waterAlpha, 0.01f, 0.f, 1.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            bool texChanged = ImGui::Checkbox("Use Texture", &w->useTexture);
            if (texChanged) {
                w->useTexture = !w->useTexture;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                w->useTexture = !w->useTexture;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Dimensions")) {
            ImGui::DragFloat("Width", &w->width, 0.1f, 0.f, 1000.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Height", &w->height, 0.1f, 0.f, 1000.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragInt("Subdivisions", &w->subdivisions, 1, 2, 200);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Physics")) {
            bool triggerChanged = ImGui::Checkbox("Is Trigger", &w->isTrigger);
            if (triggerChanged) {
                w->isTrigger = !w->isTrigger;
                if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
                w->isTrigger = !w->isTrigger;
                if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
            }

            ImGui::DragFloat("Buoyancy Force", &w->buoyancyForce, 0.5f, 0.f, 500.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::DragFloat("Water Drag", &w->waterDrag, 0.001f, 0.f, 1.f);
            if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::TreeNode("Water Info")) {
            ImGui::Text("Wave Time: %.3f", w->waveTime);
            ImGui::Text("Water Texture ID: %u", w->waterTexture);
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Water Body", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));
                DebugLog::addMessage("Removed Water Body\n");
            }
        }
    }
}


void InspectorWindow::renderWeapon(GameObject* selected) {
    WeaponComponent* w = selected->getComponent<WeaponComponent>();
    const Editor::ComponentEntry* entry = Editor::findEntry("Weapon");

    if (ImGui::CollapsingHeader("Weapon")) {

        bool aimChanged = ImGui::Checkbox("Aim At Mouse", &w->aimAtMouse);
        if (aimChanged) {
            w->aimAtMouse = !w->aimAtMouse;
            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
            w->aimAtMouse = !w->aimAtMouse;
            if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);
        }

        ImGui::DragFloat("Barrel Offset X", &w->barrelOffset.x, 0.05f);
        if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::DragFloat("Barrel Offset Y", &w->barrelOffset.y, 0.05f);
        if (ImGui::IsItemActivated())            if (entry) UndoRedoManager::Instance().beginComponentEdit(selected, *entry);
        if (ImGui::IsItemDeactivatedAfterEdit()) if (entry) UndoRedoManager::Instance().endComponentEdit(m_manager, selected, *entry);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // debug info
        if (ImGui::TreeNode("Weapon Info")) {
            ImGui::Text("Current Angle: %.2f", w->currentAngle);
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (entry) {
            if (ImGui::Button("Remove Weapon", ImVec2(-1, 0))) {
                UndoRedoManager::Instance().executeCmd(
                    std::make_unique<RemoveComponentCmd>(m_manager, selected, *entry));
                DebugLog::addMessage("Removed Weapon\n");
            }
        }
    }
}

void InspectorWindow::renderLayerControls(GameObject* selected) {
    if (ImGui::CollapsingHeader("Layer")) {
        int currentLayer = selected->getLayer();
        Layer* layer = m_manager.getLayerManager().getLayer(currentLayer);

        /* ------------------- Layer Properties ------------------- */
        if (layer && ImGui::TreeNode("Layer Properties")) {

            // layers dropdown
            if (ImGui::BeginCombo("Assign to Layer", layer->getLayerName().c_str())) {
                std::vector<Layer*> allLayers = m_manager.getLayerManager().getAllLayers();

                for (Layer* l : allLayers) {
                    bool isSelected = (l->getLayerID() == currentLayer);
                    std::string label = l->getLayerName() + " (ID: " + std::to_string(l->getLayerID()) + ", Objects: " + std::to_string(l->getObjectCount()) + ")";

                    if (ImGui::Selectable(label.c_str(), isSelected)) {
                        UndoRedoManager::Instance().beginLayerAssign(selected);\

                        if (m_manager.assignObjectToLayer(selected, l->getLayerID())) {
                            UndoRedoManager::Instance().endLayerAssign(m_manager, selected, l->getLayerID());

                            DebugLog::addMessage(selected->getObjectName() + " moved to layer: " + l->getLayerName());
                        }
                        else {
                            DebugLog::addMessage("Failed to assign layer!");
                        }
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // master flag
            bool enabled = layer->isEnabled();
            if (ImGui::Checkbox("Enabled", &enabled)) {
                UndoRedoManager::Instance().beginLayerEdit(currentLayer, layer);

                layer->setEnabled(enabled);

                UndoRedoManager::Instance().endLayerEdit(m_manager, layer);

                DebugLog::addMessage("Layer " + layer->getLayerName() +
                    (enabled ? " enabled" : " disabled"));
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Master toggle - affects both visibility and updates");
            }

            // flags
            ImGui::BeginDisabled(!enabled);

            bool visible = layer->isVisible();
            if (ImGui::Checkbox("Visible", &visible)) {
                UndoRedoManager::Instance().beginLayerEdit(currentLayer, layer);

                layer->setVisible(visible);

                UndoRedoManager::Instance().endLayerEdit(m_manager, layer);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Controls rendering only");
            }

            bool updatable = layer->isUpdatable();
            if (ImGui::Checkbox("Updatable", &updatable)) {
                UndoRedoManager::Instance().beginLayerEdit(currentLayer, layer);

                layer->setUpdatable(updatable);

                UndoRedoManager::Instance().endLayerEdit(m_manager, layer);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Controls physics/logic updates only");
            }

            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (layer && ImGui::TreeNode("Layer Info")) {
            // Rename field — commits on Enter or focus loss (Unity-style)
            static char layerNameBuf[64] = {};
            static int  lastEditedLayerID = -1;

            // Sync buffer if we switched to a different layer
            if (lastEditedLayerID != currentLayer) {
                strncpy_s(layerNameBuf, layer->getLayerName().c_str(), sizeof(layerNameBuf) - 1);
                lastEditedLayerID = currentLayer;
            }

            ImGui::SetNextItemWidth(160.f);
            ImGui::InputText("Name##layerRename", layerNameBuf, sizeof(layerNameBuf));
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                std::string newName(layerNameBuf);
                if (!newName.empty() && newName != layer->getLayerName()) {
                    m_manager.getLayerManager().renameLayer(currentLayer, newName);
                    DebugLog::addMessage("Layer " + std::to_string(currentLayer) + " renamed to: " + newName);
                }
            }

            ImGui::Text("ID: %d", currentLayer);
            ImGui::Text("Objects in layer: %zu", layer->getObjectCount());
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::Spacing();
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