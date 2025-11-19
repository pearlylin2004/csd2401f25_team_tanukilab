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
        }
        else {
            ImGui::Text("Prefab Name: "); ImGui::SameLine();
            ImGui::Text(selected->getObjectName().c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ------------------- Object Transformation ------------------- */
        if (Transform* transform = selected->getComponent<Transform>()) {
            CollisionInfo* collision = selected->getComponent<CollisionInfo>();
            renderTransform(transform, collision);
        }
        /* ------------------- END ------------------- */

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ------------------- Render ------------------- */
        if (Render* render = selected->getComponent<Render>()) {
            Animation* animation = selected->getComponent<Animation>();
            renderRender(render, selected, animation);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Collision ------------------- */
        if (CollisionInfo* collision = selected->getComponent<CollisionInfo>()) {
            Transform* transform = selected->getComponent<Transform>();
            renderCollision(collision, transform, selected);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Input Control ------------------- */
        if (selected->hasComponent<Input>()) {
            renderInput();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Physics ------------------- */
        if (Physics* physics = selected->getComponent<Physics>()) {
            //Transform* transform = selected->getComponent<Transform>();
            renderPhysics(physics, selected->hasComponent<Input>());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
        /* ------------------- END ------------------- */


        /* ------------------- Font ------------------- */
        if (FontComponent* font = selected->getComponent<FontComponent>()) {
            //Transform* transform = selected->getComponent<Transform>();
            renderFont(font);

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
    }

    ImGui::End();
}

void InspectorWindow::addComponent(GameObject* selected) {
    //if (!ImGui::CollapsingHeader("Add Component")) return;

    ImGui::BeginChild("AddComponentSection", ImVec2(0, 300), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("Add Component");
    ImGui::Separator();

    static char search[64] = "";
    ImGui::InputTextWithHint("##search", "Search components...", search, sizeof(search));

    for (const Editor::ComponentEntry& comp : Editor::componentRegistry) {
        // skip if selected obj alr has this component
        if (comp.hasComp(selected)) continue;

        // search
        if (sizeof(search) > 0) {
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

void InspectorWindow::renderTransform(Transform* transform, CollisionInfo* collision) {
    if (ImGui::CollapsingHeader("Transform")) {

        if (ImGui::TreeNode("Position")) {
            ImGui::DragFloat("X", &transform->x, 0.1f);
            ImGui::DragFloat("Y", &transform->y, 0.1f);

            // set z to stay within the editor camera (-1.f <= z < 1.f)
            glm::mat4& proj = renderer::editorCam.proj;
            float m22 = proj[2][2];
            float m32 = proj[3][2];
            float near1 = (m32 - 1.f) / m22;
            float far1 = (m32 + 1.f) / m22;
            ImGui::DragFloat("Z", &transform->z, 0.1f, far1, near1);

            ImGui::TreePop();
        }

        /* ------------------- Object Rotation ------------------- */
        if (ImGui::TreeNode("Rotation")) {
            ImGui::DragFloat("Angle", &transform->rotation);

            ImGui::TreePop();
        }

        /* ------------------- Object Scale ------------------- */
        if (ImGui::TreeNode("Scale")) {
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

            if (collision) {
                if (collision->autoFitScale) {
                    collision->colliderSize.x = transform->scaleX;
                    collision->colliderSize.y = transform->scaleY;
                }
            }

            ImGui::TreePop();
        }
    }
}

void InspectorWindow::renderRender(Render* render, GameObject* selected, Animation* animation) {
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
                    render->texChanged = false;
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
                    render->modelRef.shape = shape::triangle; // oops triangle is white circle actually
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
        
        
        if (ImGui::TreeNode("Animation")) {
            if (!animation) {
                ImGui::Spacing();
                if (ImGui::Selectable("Add Animation")) {
                    selected->addComponent<Animation>();
                }
            }
            else if (render->hasTex) {
                ImGui::Checkbox("Is animated", &render->hasAnimation);

                ImGui::Separator();

                ImGui::BeginDisabled(!render->hasAnimation);

                ImGui::Checkbox("Loop (WIP)", &animation->loop);

                ImGui::InputInt("Column", &animation->totalColumn);
                animation->totalColumn = std::max(animation->totalColumn, 1);

                ImGui::InputInt("Row", &animation->totalRow);
                animation->totalRow = std::max(animation->totalRow, 1);

                ImGui::InputFloat("Frame Time", &animation->frameTime, 0.1f);


                ImGui::EndDisabled();
            }
            else {
                ImGui::TextWrapped("Add a texture to Render to enable animation");
            }

            ImGui::TreePop();
        }


        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove Component (WIP)", ImVec2(-1, 0))) {
            // add a delete component func here

            DebugLog::addMessage("Removed Render Component");
        }
    }
}

void InspectorWindow::renderCollision(CollisionInfo* collision, Transform* transform, GameObject* selected) {
    if (ImGui::CollapsingHeader("Collision")) {
        {
            // checked means this object can collide with obj in the same layer
            ImGui::Checkbox("Enable", &collision->collisionFlag);
            
            // WIP, select layer
            /*int layer = selected->getLayer();
            if (ImGui::InputInt("Layer", &layer)) {
                if (layer < 0) layer = 0;
                
                selected->setLayer(layer);

                DebugLog::addMessage(selected->getObjectName() + " changed layer to " + std::to_string(selected->getLayer()));
            }*/
        }

        ImGui::Spacing();
        ImGui::Separator();

        {
            if (ImGui::TreeNode("Collider")) {
                // cannot edit if obj cannot collide
                ImGui::BeginDisabled(!collision->collisionFlag);
                ImGui::Checkbox("Auto-fit Scale", &collision->autoFitScale);
                ImGui::EndDisabled();

                // cannot edit if obj cannot collide or auto-fit collider size with obj size is on
                bool editableCollider = collision->collisionFlag && !collision->autoFitScale;
                ImGui::BeginDisabled(!editableCollider);
                ImGui::InputFloat("Width", &collision->colliderSize.x, 1.f);
                ImGui::InputFloat("Height", &collision->colliderSize.y, 1.f);
                ImGui::EndDisabled();

                ImGui::TreePop();
            }
        }

    }
}

void InspectorWindow::renderInput() {
    if (ImGui::CollapsingHeader("Input Control")) {
        ImGui::TextWrapped("This object can be controlled by keyboard input");
    }
}

void InspectorWindow::renderPhysics(Physics* physic, bool canMove) {
    if (ImGui::CollapsingHeader("Physics")) {
        ImGui::Checkbox("Is Trigger", &physic->physicsFlag);

        ImGui::Spacing();
        ImGui::Separator();
       

        // only show movement data if object is controlable via input
        if (ImGui::TreeNode("Input Movement Control")) {
            if (canMove) {
                if (ImGui::TreeNode("Movement")) {
                    ImGui::InputFloat("Speed", &physic->moveSpeed, 1.f);
                    physic->moveSpeed = std::max(physic->moveSpeed, 0.f); // set the min speed to 0.f

                    ImGui::InputFloat("Jump Force", &physic->jumpForce, 1.f);
                    physic->jumpForce = std::max(physic->jumpForce, 0.f); // set the min jump force to 0.f

                    ImGui::InputFloat("Mass", &physic->dynamics.mass, 1.f);
                    physic->dynamics.mass = std::max(physic->dynamics.mass, 0.f); // 0.f mass means static obj (never move, e.g. wall)

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Environment")) {
                    ImGui::InputFloat("Damping", &physic->damping, 0.1f);
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
    }
}

void InspectorWindow::renderFont(FontComponent* font) {
    if (ImGui::CollapsingHeader("Font")) {
        {
            std::string& objName = font->word;
            char buffer[128];
            strncpy(buffer, objName.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText("##Word", buffer, sizeof(buffer))) {
                font->word = buffer;
            }

            ImGui::InputFloat("Size", &font->scale, 1.f);
            font->scale = std::max(font->scale, 0.f);

            ImGui::ColorEdit3("Colour", &font->clr.x);

            /* scuffed way to select font */
            static const char* fontNames[] = {
                "Orange Knight",
                "Arial",
                "Times New Roman"
            };

            //int fontCount = IM_ARRAYSIZE(fontNames);
            int currentFont = font->fontType;

            if (ImGui::Combo("Font Type", &font->fontType, fontNames, IM_ARRAYSIZE(fontNames))) {
                //font->fontType = currentFont;
            }
        }

    }
}