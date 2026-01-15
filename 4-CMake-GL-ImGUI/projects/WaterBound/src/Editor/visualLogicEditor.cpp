/* Start Header ************************************************************************/
/*!
\file       visualLogicEditor.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       January, 15th, 2026
\brief      s

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifdef _DEBUG
#include "Editor/editorManager.h"

VisualLogicEditor::VisualLogicEditor() :
	m_nextNodeId(1),
	m_selectedNodeId(-1),
	m_isDragging(false),
	m_draggingNodeId(-1),
	m_canvasOffset(50.0f, 50.0f),
	m_zoom(1.0f)
{ }

void VisualLogicEditor::render() {
	ImGui::Begin("Visual Logic Editor", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Add Node")) {
			if (ImGui::MenuItem("On Start Event")) {
				addEventNode(ImVec2(100, 100));
			}

			if (ImGui::MenuItem("Print Action")) {
				addActionNode(ImVec2(100, 200));
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Reset View")) {
				m_canvasOffset = ImVec2(50, 50);
				m_zoom = 1.f;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	// canvas area
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();

	ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
	bool isCanvasHovered = ImGui::IsItemHovered();

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

	// grid background
	const float GRIDSIZE = 64.f;
	const ImU32 GRIDCOLOUR = IM_COL32(60, 60, 60, 255);

	for (float x = fmodf(m_canvasOffset.x, GRIDSIZE); x < canvasSize.x; x += GRIDSIZE) {
		drawList->AddLine(
			ImVec2(canvasPos.x + x, canvasPos.y),
			ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
			GRIDCOLOUR);
	}

	for (float y = fmodf(m_canvasOffset.y, GRIDSIZE); y < canvasSize.y; y += GRIDSIZE) {
		drawList->AddLine(
			ImVec2(canvasPos.x, canvasPos.y + y),
			ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
			GRIDCOLOUR);
	}

	// render all nodes
	for (auto& node : m_nodes) {
		renderNode(drawList, node, canvasPos, isCanvasHovered);
	}

	// pan with middle mouse
	if (isCanvasHovered && InputHandler::isMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)) {
		Vector2D delta = InputHandler::getMouseDelta();
		m_canvasOffset.x += delta.x;
		m_canvasOffset.y += delta.y;
	}

	// stop dragging when mouse release
	if (InputHandler::isMouseLeftReleased()) {
		m_isDragging = false;
		m_draggingNodeId = -1;
	}

	if (isCanvasHovered && InputHandler::getMouseScroll() != 0.f) {
		float scroll = InputHandler::getMouseScroll();
		m_zoom += scroll * 0.1f;
		m_zoom = glm::clamp(m_zoom, 0.5f, 2.0f);
	}

	drawList->PopClipRect();

	// Info panel
	ImGui::Text("Nodes: %d", (int)m_nodes.size());
	ImGui::Text("Selected: %d", m_selectedNodeId);
	ImGui::Text("Zoom: %.1f", m_zoom);

	ImGui::End();
}

void VisualLogicEditor::renderNode(ImDrawList* drawList, Editor::LogicNode& node, ImVec2 canvasPos, bool isCanvasHovered) {
	// calc screen position
	ImVec2 nodePos = ImVec2(canvasPos.x + node.position.x + m_canvasOffset.x,
		canvasPos.y + node.position.y + m_canvasOffset.y);

	ImVec2 nodeSize(250.f, 80.f);

	// check if mouse if hovering
	ImVec2 mousePos = ImGui::GetMousePos();
	bool isHovered = isCanvasHovered && 
		mousePos.x >= nodePos.x && mousePos.x <= nodePos.x + nodeSize.x &&
		mousePos.y >= nodePos.y && mousePos.y <= nodePos.y + nodeSize.y;

	// handle selection and dragging
	if (isHovered && InputHandler::isMouseLeftClicked()) {
		m_selectedNodeId = node.id;
		m_isDragging = true;
		m_draggingNodeId = node.id;
		m_dragOffset = ImVec2(mousePos.x - nodePos.x, mousePos.y - nodePos.y);
	}

	// update position if dragging
	if (m_isDragging && m_draggingNodeId == node.id && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		node.position.x = mousePos.x - canvasPos.x - m_canvasOffset.x - m_dragOffset.x;
		node.position.y = mousePos.y - canvasPos.y - m_canvasOffset.y - m_dragOffset.y;
	}

	// draw node background
	ImU32 nodeColor = getNodeColor(node.type);
	ImU32 borderColor = (m_selectedNodeId == node.id) ?
		IM_COL32(255, 255, 0, 255) : IM_COL32(100, 100, 100, 255);
	float borderThickness = (m_selectedNodeId == node.id) ? 3.f : 1.f;

	drawList->AddRectFilled(nodePos,
		ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y),
		nodeColor, 4.f);

	drawList->AddRect(nodePos,
		ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y),
		borderColor, 4.f, 0, borderThickness);

	// node name
	drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 10),
		IM_COL32(255, 255, 255, 255), node.name.c_str());

	// draw input pin (if have)
	if (node.hasInput) {
		ImVec2 pinPos(nodePos.x, nodePos.y + 30);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(200, 200, 200, 255));
		drawList->AddText(ImVec2(pinPos.x + 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "In");
	}

	// draw output pin (if have)
	if (node.hasOutput) {
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + 30);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(200, 200, 200, 255));

		ImVec2 textSize = ImGui::CalcTextSize("Out");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "Out");
	}

	// delete button (show on hover)
	if (isHovered) {
		ImVec2 deletePos(nodePos.x + nodeSize.x - 25, nodePos.y + 5);
		ImVec2 deleteSize(20, 20);

		bool deleteHovered = mousePos.x >= deletePos.x && mousePos.x <= deletePos.x + deleteSize.x &&
			mousePos.y >= deletePos.y && mousePos.y <= deletePos.y + deleteSize.y;

		ImU32 deleteColor = deleteHovered ? IM_COL32(255, 80, 80, 255) : IM_COL32(200, 60, 60, 255);

		drawList->AddRectFilled(deletePos,
			ImVec2(deletePos.x + deleteSize.x, deletePos.y + deleteSize.y),
			deleteColor, 2.f);

		drawList->AddText(ImVec2(deletePos.x + 6, deletePos.y + 2),
			IM_COL32(255, 255, 255, 255), "X");

		// Handle delete click
		if (deleteHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			// Mark for deletion (we'll remove it after the loop)
			m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(),
				[&node](const Editor::LogicNode& n) { return n.id == node.id; }),
				m_nodes.end());

			if (m_selectedNodeId == node.id) {
				m_selectedNodeId = -1;
			}
		}
	}
}

void VisualLogicEditor::addEventNode(ImVec2 position) {
	Editor::LogicNode node;
	node.id = m_nextNodeId++;
	node.name = "On Start";
	node.type = Editor::NodeType::EVENT;
	node.position = position;
	node.hasInput = false;
	node.hasOutput = true;

	m_nodes.push_back(node);

}

void VisualLogicEditor::addActionNode(ImVec2 position) {
	Editor::LogicNode node;
	node.id = m_nextNodeId++;
	node.name = "Print Message";
	node.type = Editor::NodeType::ACTION;
	node.position = position;
	node.hasInput = true;
	node.hasOutput = true;

	m_nodes.push_back(node);
}

ImU32 VisualLogicEditor::getNodeColor(Editor::NodeType type) {
	switch (type) {
	case Editor::NodeType::EVENT:
		return IM_COL32(76, 175, 80, 255);  // green
	case Editor::NodeType::ACTION:
		return IM_COL32(33, 150, 243, 255); // blue
	default:
		return IM_COL32(128, 128, 128, 255); // gray
	}
}

#endif