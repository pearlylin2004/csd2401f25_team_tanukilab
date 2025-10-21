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

UI::UI() {
	// temp, get ready to load from json in the future
    m_shapePresets = {
		{ ShapeType::Leaves, { 0.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f,
		{-20.f, 20.f}, {-20.f, 20.f}, {0.f, 100.f}, { 0.1f, 5.f }, { 0.1f, 5.f } } },
		{ ShapeType::Bullet, { 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f } }
	};
}

void UI::update(GameObjectManager& manager) {
    drawPerformance();
    drawShapes(manager);
    drawDebug();
    objInspectWindow(manager);
}

void UI::addDebugMessage(const std::string& msg) {
    m_debugMessages.push_back(msg);
}

// edit shape windows
void UI::shapeControl(Transform& t1, Transform& t2) {
    if (ImGui::Begin("Shape Control")) {
        ImGui::Text("Transform Component");
        static ShapeType currentShape = ShapeType::Leaves;

        if (ImGui::Button("Leaves")) {
            currentShape = ShapeType::Leaves;
        }
        ImGui::SameLine();
        if (ImGui::Button("Bullet")) {
            currentShape = ShapeType::Bullet;
        }

        //check which shape is selected
        Transform* t = (currentShape == ShapeType::Leaves) ? &t1 : &t2;
        ShapeDefaults& defaults = m_shapePresets[currentShape];

        if (ImGui::CollapsingHeader("Transform")) {
            if (ImGui::TreeNode("Position")) {
                ImGui::SliderFloat("X", &t->x, defaults.posXRange.min, defaults.posXRange.max);
                ImGui::SliderFloat("Y", &t->y, defaults.posYRange.min, defaults.posYRange.max);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Rotation")) {
                ImGui::SliderFloat("Angle", &t->rotation, defaults.rotRange.min, defaults.rotRange.max);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Scale")) {
                ImGui::SliderFloat("X", &t->scaleX, defaults.scaleXRange.min, defaults.scaleXRange.max);
                ImGui::SliderFloat("Y", &t->scaleY, defaults.scaleYRange.min, defaults.scaleYRange.max);
                ImGui::SliderFloat("Z", &t->scaleZ, 0.1f, 10.f);
                ImGui::TreePop();
            }
        }

        //reset
        if (ImGui::Button("Reset All")) {
            if (m_shapePresets.find(currentShape) != m_shapePresets.end()) {
                t->x = defaults.posX;
                t->y = defaults.posY;
                t->rotation = defaults.rotation;
                t->scaleX = defaults.scaleX;
                t->scaleY = defaults.scaleY;
                t->scaleZ = defaults.scaleZ;
            }
            else {
                m_debugMessages.push_back("No presets found for the selected shape.");
            }
        }
    }
    ImGui::End();
}

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

void UI::objListPanel(GameObjectManager& manager) {
    std::vector<const char*> names; // adjust max size

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    names.reserve(gameObjects.size());
    for (int i = 0; i < gameObjects.size(); ++i) {
        names.push_back(gameObjects[i]->getObjectName().c_str());
    }

    if (!names.empty()) {
        if (ImGui::ListBox("Objects", &m_ObjUIState.selectedIndex, names.data(), static_cast<int>(names.size()), m_ObjUIState.maxObjects)) {
            m_ObjUIState.selectedObject = gameObjects[m_ObjUIState.selectedIndex];
        }
    }
    else {
        m_ObjUIState.selectedObject = nullptr;
        ImGui::Text("No GameObjects available");
    }
}

void UI::objPropertyPanel(GameObjectManager& manager) {
    if (m_ObjUIState.selectedObject) {
		// object name edit
        std::string& objName = m_ObjUIState.selectedObject->getObjectName();
        char buffer[256];
        strncpy(buffer, objName.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Name");
		ImGui::SameLine();
        if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
            objName = buffer;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


		// object transform properties
        Transform* transform = m_ObjUIState.selectedObject->getComponent<Transform>();
        if (ImGui::CollapsingHeader("Transform")) {
            ImGui::DragFloat("X", &transform->x);
            ImGui::DragFloat("Y", &transform->y);
            ImGui::DragFloat("Z", &transform->z);
        }

        if (ImGui::CollapsingHeader("Rotation")) {
            ImGui::DragFloat("Angle", &transform->rotation);
        }
            
        if (ImGui::CollapsingHeader("Scale")) {
            ImGui::DragFloat("X", &transform->scaleX);
            ImGui::DragFloat("Y", &transform->scaleY);
            ImGui::DragFloat("Z", &transform->scaleZ);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        

		// delete object button (NOT WORKING YET)
        if (ImGui::Button("Delete Object (not available)")) {
			// DELETE OBJECT FROM MANAGER, add in the actual delete func once available

            m_ObjUIState.selectedObject = nullptr;
            m_ObjUIState.selectedIndex = -1;
        }

    }
    else {
        ImGui::Text("No object selected");
    }
}

void UI::objInspectWindow(GameObjectManager& manager) {
    ImGui::Begin("Object Properties Editor");

	// Left panel - object list
    ImGui::BeginChild("Object List", ImVec2(m_ObjUIState.objListWidth, 0));
    objListPanel(manager);
    ImGui::EndChild();

    ImGui::SameLine();

	// splitter that allow resizing
    ImGui::InvisibleButton("splitter", ImVec2(m_ObjUIState.splitterSize, m_ObjUIState.objListHeight));
    if (ImGui::IsItemActive()) {
        m_ObjUIState.objListWidth += InputHandler::getMouseDelta().x;
        m_ObjUIState.objListWidth = std::clamp(m_ObjUIState.objListWidth, m_ObjUIState.objListMinWidth, m_ObjUIState.objListMaxWidth);
    }
	// draw the vertical line to indicate the splitter
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 splitterMin = ImGui::GetItemRectMin();
    ImVec2 splitterMax = ImGui::GetItemRectMax();
    draw_list->AddLine(
        ImVec2(splitterMin.x + m_ObjUIState.splitterSize * 0.5f, splitterMin.y),
        ImVec2(splitterMin.x + m_ObjUIState.splitterSize * 0.5f, splitterMax.y),
        IM_COL32(100, 100, 100, 255), 1.0f
    );

    ImGui::SameLine();

    // Right panel — inspector
    ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
    objPropertyPanel(manager);
    ImGui::EndChild();

    ImGui::End();
}
