/* Start Header ************************************************************************/
/*!
\file       visualLogicEditor.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       January, 15th, 2026
\brief      s

Copyright (C) 2026 DigiPen Institute of Technology.
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
	m_canvasOffset(50.f, 50.f),
	m_zoom(1.f),
	m_isConnecting(false),
	m_connectingFromNodeId(-1),
	m_saveFilename{0}
{ }

void VisualLogicEditor::render() {
	if (!Editor::visualLogicState.showEditor) return;

	ImGui::Begin("Visual Logic Editor", &Editor::visualLogicState.showEditor, ImGuiWindowFlags_MenuBar);

	Editor::visualLogicState.isFocus = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::BeginMenuBar()) {
		/* -------- File -------- */
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				saveLogic();
			}

			if (ImGui::MenuItem("Save and Close")) {
				saveLogic();
				Editor::visualLogicState.showEditor = false;
				m_selectedNodeId = -1;
			}

			ImGui::EndMenu();
		}
		/* -------- END -------- */


		/* -------- Nodes -------- */
		if (ImGui::BeginMenu("Add Node")) {
			/* -------- Events -------- */
			if (ImGui::BeginMenu("Events")) {
				if (ImGui::MenuItem("On Start")) {
					addNode(Vector2D(100, 100), NodeType::EVENT, NodeImplementation::ON_START);
				}

				if (ImGui::MenuItem("On Update")) {
					addNode(Vector2D(150, 200), NodeType::EVENT, NodeImplementation::ON_UPDATE);
				}

				if (ImGui::MenuItem("On Collision (WIP)")) {
					addNode(Vector2D(200, 300), NodeType::EVENT, NodeImplementation::ON_COLLISION);
				}

				ImGui::EndMenu();
			}
			/* -------- END -------- */


			/* -------- Conditions -------- */
			if (ImGui::BeginMenu("Conditions")) {
				if (ImGui::MenuItem("Is Key Held")) {
					addNode(Vector2D(350, 100), NodeType::CONDITION, NodeImplementation::CHECK_KEY_HELD);
				}

				if (ImGui::MenuItem("Compare Value")) {
					addNode(Vector2D(400, 100), NodeType::CONDITION, NodeImplementation::COMPARE_VALUE);
				}

				if (ImGui::MenuItem("Compare Variable")) {
					addNode(Vector2D(460, 100), NodeType::CONDITION, NodeImplementation::COMPARE_VAR);
				}

				ImGui::EndMenu();
			}
			/* -------- END -------- */
			

			/* -------- Actions -------- */
			if (ImGui::BeginMenu("Actions")) {
				if (ImGui::MenuItem("Print Message")) {
					addNode(Vector2D(250, 100), NodeType::ACTION, NodeImplementation::PRINT_MESSAGE);
				}

				if (ImGui::MenuItem("Move Object")) {
					addNode(Vector2D(300, 200), NodeType::ACTION, NodeImplementation::MOVE_OBJECT);
				}

				if (ImGui::MenuItem("Rotate Object")) {
					addNode(Vector2D(350, 200), NodeType::ACTION, NodeImplementation::ROTATE_OBJECT);
				}

				if (ImGui::MenuItem("Set Float")) {
					addNode(Vector2D(400, 200), NodeType::ACTION, NodeImplementation::SET_FLOAT);
				}

				ImGui::EndMenu();
			}
			/* -------- END -------- */

			ImGui::EndMenu();
		}
		/* -------- END -------- */


		/* -------- View -------- */
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Reset View")) {
				m_canvasOffset = ImVec2(50, 50);
				m_zoom = 1.f;
			}

			ImGui::EndMenu();
		}
		/* -------- END -------- */

		ImGui::EndMenuBar();
	}

	/* ------ Canvas ------ */
	ImGui::BeginChild("Canvas", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 0), true);

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
		drawList->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
			ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
			GRIDCOLOUR);
	}

	for (float y = fmodf(m_canvasOffset.y, GRIDSIZE); y < canvasSize.y; y += GRIDSIZE) {
		drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
			ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
			GRIDCOLOUR);
	}

	// render connections (behind nodes)
	renderConnections(drawList, canvasPos);

	// render all nodes
	for (LogicNode& node : Editor::visualLogicState.nodes) {
		renderNode(drawList, node, canvasPos, isCanvasHovered);
	}

	// draw line when creating connection
	if (m_isConnecting) {
		ImVec2 mousePos = ImGui::GetMousePos();
		drawList->AddLine(m_connectionStartPos, mousePos, IM_COL32(255, 255, 0, 255), 3.f);
	}

	// cancel connection with right click
	if (m_isConnecting && InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_RIGHT)) {
		m_isConnecting = false;
		m_connectingFromNodeId = -1;
		m_connectingFromPin = "";
	}

	// click empty space to deselect
	if (isCanvasHovered && InputHandler::isMouseLeftClicked() && !m_isConnecting) {
		bool clickedOnNode = false;
		bool clickedOnCon = false;

		ImVec2 mousePos = ImGui::GetMousePos();

		/* -------- Check for Nodes -------- */
		for (LogicNode& node : Editor::visualLogicState.nodes) {
			ImVec2 nodePos = ImVec2(canvasPos.x + node.position.x + m_canvasOffset.x,
				canvasPos.y + node.position.y + m_canvasOffset.y);
			ImVec2 nodeSize(250.f, 80.f);

			// if click on any nodes
			if (mousePos.x >= nodePos.x && mousePos.x <= nodePos.x + nodeSize.x &&
				mousePos.y >= nodePos.y && mousePos.y <= nodePos.y + nodeSize.y) {
				clickedOnNode = true;
				break;
			}
		}
		/* -------- END -------- */

		/* -------- Check for Connections -------- */
		if (!clickedOnNode) {
			for (size_t i = 0; i < Editor::visualLogicState.connections.size(); ++i) {
				const LogicConnection& conn = Editor::visualLogicState.connections[i];

				LogicNode* fromNode = nullptr;
				LogicNode* toNode = nullptr;

				for (LogicNode& node : Editor::visualLogicState.nodes) {
					if (node.id == conn.fromNodeId) fromNode = &node;
					if (node.id == conn.toNodeId) toNode = &node;
				}

				if (!fromNode || !toNode) continue;

				ImVec2 fromPos = ImVec2(canvasPos.x + fromNode->position.x + m_canvasOffset.x + 250.f,
					canvasPos.y + fromNode->position.y + m_canvasOffset.y + 40.f);

				if (conn.outputPin == "True") fromPos.y -= 10.f;
				if (conn.outputPin == "False") fromPos.y += 10.f;

				ImVec2 toPos = ImVec2(canvasPos.x + toNode->position.x + m_canvasOffset.x,
					canvasPos.y + toNode->position.y + m_canvasOffset.y + 40.f);

				ImVec2 cp1(fromPos.x + 50, fromPos.y);
				ImVec2 cp2(toPos.x - 50, toPos.y);

				if (isMouseNearBezier(fromPos, cp1, cp2, toPos, mousePos, 10.f)) {
					clickedOnCon = true;
					break;
				}
			}
		}
		/* -------- Check for Nodes -------- */

		// deselect if click on empty space
		if (!clickedOnNode && !clickedOnCon) {
			m_selectedNodeId = -1;
			m_selectedConIndex = -1;
		}
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
		m_zoom = glm::clamp(m_zoom, 0.5f, 2.f);
	}

	// shortcut
	if (Editor::visualLogicState.isFocus) {
		// delete node/con with DEL / Backspace
		if ((InputHandler::isKeyTriggered(GLFW_KEY_DELETE) ||
			InputHandler::isKeyTriggered(GLFW_KEY_BACKSPACE))) {
			if (m_selectedNodeId != -1) {
				deleteSelectedNode();
			}
			else if (m_selectedConIndex != -1) {
				deleteSelectedConnection();
			}
		}

		// duplicate node with Ctrl+D
		if (InputHandler::isComboKeyTriggered(GLFW_KEY_D) &&
			m_selectedNodeId != -1) {
			duplicateSelectedNode();
		}

		// select next node with Tab, previous with Shift+Tab
		if (InputHandler::isKeyTriggered(GLFW_KEY_TAB)) {
			bool forward = !InputHandler::isKeyHeld(GLFW_KEY_LEFT_SHIFT);
			selectNextNode(forward);
		}
	}

	drawList->PopClipRect();

	// Info panel
	if (Editor::visualLogicState.targetObj) {
		ImGui::Text("Selected Object: %s", Editor::visualLogicState.targetObj->getObjectName().c_str());
	}
	ImGui::Text("Nodes: %d", (int)Editor::visualLogicState.nodes.size());

	if (m_selectedNodeId != -1) {
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Node selected");
	}
	else if (m_selectedConIndex != -1) {
		ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Connection selected");
	}

	ImGui::EndChild();

	/* ------ END ------ */


	/* ------ Properties Panel ------ */
	ImGui::SameLine();
	ImGui::BeginChild("Properties", ImVec2(0, 0), true);
	renderNodeProperties();
	ImGui::EndChild();
	/* ------ END ------ */


	ImGui::End();


	// popup
	renderSavePopup();
}

void VisualLogicEditor::updateNextNodeId() {
	m_nextNodeId = 1;
	for (const LogicNode& node : Editor::visualLogicState.nodes) {
		if (node.id >= m_nextNodeId) {
			m_nextNodeId = node.id + 1;
		}
	}
}

// In VisualLogicEditor class
void VisualLogicEditor::clear() {
	Editor::visualLogicState.nodes.clear();
	Editor::visualLogicState.connections.clear();
	Editor::visualLogicState.targetObj = nullptr;
	m_selectedNodeId = -1;
	m_selectedConIndex = -1;
	m_isDragging = false;
	m_draggingNodeId = -1;
	m_isConnecting = false;
	m_connectingFromNodeId = -1;
	m_nextNodeId = 1;
}

void VisualLogicEditor::saveLogic() {
	if (!Editor::visualLogicState.targetObj) return;
	VisualLC* logic = Editor::visualLogicState.targetObj->getComponent<VisualLC>();
	if (!logic) return;

	if (logic->filename.empty()) {
		Editor::visualLogicState.showSavePopup = true;
		return;
	}

	std::string filepath = "/Logic/" + logic->filename;

	logic->nodes = Editor::visualLogicState.nodes;
	logic->connections = Editor::visualLogicState.connections;

	std::string err;
	if (JsonIO::saveVisualLogic(filepath, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {
		DebugLog::addMessage("Saved Visual Logic: " + filepath);
	}
	else {
		DebugLog::addMessage("Failed to save: " + err);
	}
}

//void VisualLogicEditor::loadLogic(const std::string& filename) {
//	std::string filepath = "/Logic/" + filename;
//
//	std::string err;
//	if (JsonIO::loadVisualLogic(filepath, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {
//		DebugLog::addMessage("Loaded Visual Logic: " + filepath);
//	}
//	else {
//		DebugLog::addMessage("Failed to load: " + err);
//	}
//}

void VisualLogicEditor::renderNode(ImDrawList* drawList, LogicNode& node, ImVec2 canvasPos, bool isCanvasHovered) {
	// calc screen position
	ImVec2 nodePos = ImVec2(canvasPos.x + node.position.x + m_canvasOffset.x,
		canvasPos.y + node.position.y + m_canvasOffset.y);

	ImVec2 nodeSize(250.f, 80.f);


	/* ---------- Mouse Control ---------- */
	// check if mouse if hovering
	ImVec2 mousePos = ImGui::GetMousePos();
	bool isHovered = isCanvasHovered && 
		mousePos.x >= nodePos.x && mousePos.x <= nodePos.x + nodeSize.x &&
		mousePos.y >= nodePos.y && mousePos.y <= nodePos.y + nodeSize.y;

	// handle selection and dragging
	if (isHovered && InputHandler::isMouseLeftClicked() && !m_isConnecting) {
		m_selectedNodeId = node.id;
		m_selectedConIndex = -1; // deselect connection
		m_isDragging = true;
		m_draggingNodeId = node.id;
		m_dragOffset = ImVec2(mousePos.x - nodePos.x, mousePos.y - nodePos.y);
	}

	// update position if dragging
	if (m_isDragging && m_draggingNodeId == node.id && InputHandler::isMouseDragging()) {
		node.position.x = mousePos.x - canvasPos.x - m_canvasOffset.x - m_dragOffset.x;
		node.position.y = mousePos.y - canvasPos.y - m_canvasOffset.y - m_dragOffset.y;
	}
	/* ---------- END ---------- */


	/* ---------- Draw Nodes ---------- */
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
		IM_COL32(255, 255, 255, 255), JsonIO::nodeImplToString(node.imp).c_str());
	/* ---------- END ---------- */


	/* ---------- Draw Input Pin ---------- */
	if (node.hasInput) {
		ImVec2 pinPos(nodePos.x, nodePos.y + 30);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(200, 200, 200, 255));
		drawList->AddText(ImVec2(pinPos.x + 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "In");

		// click input pin for connection
		if (m_isConnecting && isPinHovered(pinPos, mousePos) && InputHandler::isMouseLeftClicked()) {
			createConnection(m_connectingFromNodeId, m_connectingFromPin, node.id, "In");
			m_isConnecting = false;
		}
	}
	/* ---------- END ---------- */


	/* ---------- Draw Output Pin ---------- */
	if (node.hasOutput) {
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + 30);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(200, 200, 200, 255));

		ImVec2 textSize = ImGui::CalcTextSize("Out");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "Out");

		// click output pin to start connection
		if(!m_isConnecting && isPinHovered(pinPos, mousePos) && InputHandler::isMouseLeftClicked()) {
			m_isConnecting = true;
			m_connectingFromNodeId = node.id;
			m_connectingFromPin = "Out";
			m_connectionStartPos = pinPos;
		}
	}
	/* ---------- END ---------- */


	/* ---------- True/False Pin for Connection Nodes ---------- */
	if (node.hasTrueOutput) {
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + 30);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(100, 255, 100, 255));

		ImVec2 textSize = ImGui::CalcTextSize("True");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "True");

		if (!m_isConnecting && isPinHovered(pinPos, mousePos) && InputHandler::isMouseLeftClicked()) {
			m_isConnecting = true;
			m_connectingFromNodeId = node.id;
			m_connectingFromPin = "True";
			m_connectionStartPos = pinPos;
		}
	}

	if (node.hasFalseOutput) {
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + 50);
		drawList->AddCircleFilled(pinPos, 6.f, IM_COL32(255, 100, 100, 255));

		ImVec2 textSize = ImGui::CalcTextSize("False");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - 12, pinPos.y - 8),
			IM_COL32(255, 255, 255, 255), "False");

		if (!m_isConnecting && isPinHovered(pinPos, mousePos) && InputHandler::isMouseLeftClicked()) {
			m_isConnecting = true;
			m_connectingFromNodeId = node.id;
			m_connectingFromPin = "False";
			m_connectionStartPos = pinPos;
		}
	}
	/* ---------- END ---------- */


	/* ---------- Delete Button ---------- */
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


		// delete node
		if (deleteHovered && InputHandler::isMouseLeftClicked()) {
			deleteSelectedNode();
		}
	}
	/* ---------- END ---------- */
}

void VisualLogicEditor::renderConnections(ImDrawList* drawList, ImVec2 canvasPos) {
	ImVec2 mousePos = ImGui::GetMousePos();

	for (size_t i = 0; i < Editor::visualLogicState.connections.size(); ++i) {
		const LogicConnection& conn = Editor::visualLogicState.connections[i];

		LogicNode* fromNode = nullptr;
		LogicNode* toNode = nullptr;

		for (LogicNode& node : Editor::visualLogicState.nodes) {
			if (node.id == conn.fromNodeId) fromNode = &node;
			if (node.id == conn.toNodeId) toNode = &node;
		}

		if (!fromNode || !toNode) continue;

		ImVec2 fromPos = ImVec2(canvasPos.x + fromNode->position.x + m_canvasOffset.x + 250.f, canvasPos.y + fromNode->position.y + m_canvasOffset.y + 40.f);

		if (conn.outputPin == "True") fromPos.y -= 10.f;
		if (conn.outputPin == "False") fromPos.y += 10.f;

		ImVec2 toPos = ImVec2(canvasPos.x + toNode->position.x + m_canvasOffset.x, canvasPos.y + toNode->position.y + m_canvasOffset.y + 40.f);

		// draw curve
		ImVec2 cp1(fromPos.x + 50, fromPos.y);
		ImVec2 cp2(toPos.x - 50, toPos.y);

		ImU32 lineColor = IM_COL32(200, 200, 200, 255);
		if (conn.outputPin == "True") lineColor = IM_COL32(100, 255, 100, 255);
		if (conn.outputPin == "False") lineColor = IM_COL32(255, 100, 100, 255);

		// if mouse is near curve
		bool isHovered = isMouseNearBezier(fromPos, cp1, cp2, toPos, mousePos, 10.f);

		// if connection selected
		bool isSelected = (m_selectedConIndex == static_cast<int>(i));

		float lineWidth = 2.f;
		if (isSelected) {
			lineColor = IM_COL32(255, 255, 0, 255); // yellow when selected
			lineWidth = 4.f;
		}
		else if (isHovered) {
			lineWidth = 3.5f; // slightly thicker when hovered
		}

		drawList->AddBezierCubic(fromPos, cp1, cp2, toPos, lineColor, lineWidth);

		if (isHovered && InputHandler::isMouseLeftClicked() && !m_isConnecting) {
			m_selectedConIndex = static_cast<int>(i);
			m_selectedNodeId = -1; // deselect node when connection selected
		}
	}
}

void VisualLogicEditor::renderNodeProperties() {
	if (m_selectedNodeId == -1) {
		ImGui::Text("No node selected");
		return;
	}

	std::vector<LogicNode>::iterator it = std::find_if(Editor::visualLogicState.nodes.begin(), Editor::visualLogicState.nodes.end(),
		[this](const LogicNode& n) { return n.id == m_selectedNodeId; });

	if (it == Editor::visualLogicState.nodes.end()) return;

	LogicNode& node = *it;

	ImGui::Text("Node: %s", JsonIO::nodeImplToString(node.imp).c_str());
	ImGui::Separator();

	/* ---------- ACTION ---------- */
	// print msg
	if (node.imp == NodeImplementation::PRINT_MESSAGE) {
		char buffer[256];
		strncpy(buffer, node.parameters["message"].c_str(), 255);

		if (ImGui::InputText("Message", buffer, 256)) {
			node.parameters["message"] = buffer;
		}
	}
	// move obj
	else  if (node.imp == NodeImplementation::MOVE_OBJECT) {
		float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.f;
		float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.f;

		if (ImGui::DragFloat("X Offset", &x, 0.1f)) {
			node.parameters["x"] = std::to_string(x);
		}
		if (ImGui::DragFloat("Y Offset", &y, 0.1f)) {
			node.parameters["y"] = std::to_string(y);
		}
	}
	// rotate obj
	else if (node.imp == NodeImplementation::ROTATE_OBJECT) {
		float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;

		if (ImGui::DragFloat("Angle (degrees)", &angle, 1.f)) {
			node.parameters["angle"] = std::to_string(angle);

			ImGui::Spacing();
			ImGui::TextDisabled("Positive = counter-clockwise");
		}
	}
	// set float variable
	else if (node.imp == NodeImplementation::SET_FLOAT) {
		char varBuffer[128];
		strncpy(varBuffer, node.parameters["variable"].c_str(), 127);
		varBuffer[127] = '\0';

		if (ImGui::InputText("Variable Name", varBuffer, 128)) {
			node.parameters["variable"] = varBuffer;
		}

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		if (ImGui::DragFloat("Value", &value, 0.1f)) {
			node.parameters["value"] = std::to_string(value);
		}

		ImGui::Spacing();
		ImGui::TextDisabled("Stores a float in a variable");
	}
	/* ---------- END ---------- */

	/* ---------- CONDITION ---------- */
	// check key held
	else if (node.imp == NodeImplementation::CHECK_KEY_HELD) {
		std::string currentKey = node.parameters.count("key") ?
			node.parameters["key"] : "W";

		// Dropdown for common keys
		if (ImGui::BeginCombo("Key", currentKey.c_str())) {
			const char* keys[] = {
				"W", "A", "S", "D",
				"UP", "DOWN", "LEFT", "RIGHT",
				"SPACE", "SHIFT", "CTRL", "ALT",
				"ENTER", "ESC"
			};

			for (int i = 0; i < IM_ARRAYSIZE(keys); ++i) {
				bool isSelected = (currentKey == keys[i]);

				if (ImGui::Selectable(keys[i], isSelected))
					node.parameters["key"] = keys[i];

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}
	// compare 2 values
	else if (node.imp == NodeImplementation::COMPARE_VALUE) {
		std::string property = node.parameters.count("property") ?
			node.parameters["property"] : "rotation";

		std::string comparison = node.parameters.count("comparison") ?
			node.parameters["comparison"] : "greater";

		float threshold = node.parameters.count("threshold") ?
			std::stof(node.parameters["threshold"]) : 0.f;

		// Property dropdown
		ImGui::Text("Property to check:");
		if (ImGui::BeginCombo("##property", property.c_str())) {
			const char* properties[] = {
				"rotation",
				"x", "y", "z", // position
				"scaleX", "scaleY", "scaleZ",
				"velX", "velY"
			};

			const char* labels[] = {
				"Rotation", "Position X", "Position Y", "Position Z",
				"Scale X", "Scale Y", "Scale Z",
				"Velocity X", "Velocity Y"
			};

			for (int i = 0; i < IM_ARRAYSIZE(properties); ++i) {
				bool isSelected = (property == properties[i]);

				if (ImGui::Selectable(labels[i], isSelected))
					node.parameters["property"] = properties[i];

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		// comparison dropdown
		ImGui::Text("Comparison:");
		if (ImGui::BeginCombo("##comparison", comparison.c_str())) {
			const char* comparisons[] = { "greater", "less", "equal" };
			const char* labels[] = { "Greater Than (>)", "Less Than (<)", "Equal To (=)" };

			for (int i = 0; i < 3; ++i) {
				bool isSelected = (comparison == comparisons[i]);

				if (ImGui::Selectable(labels[i], isSelected))
					node.parameters["comparison"] = comparisons[i];

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		// threshold input
		if (ImGui::DragFloat("Threshold Value", &threshold, 0.1f)) {
			node.parameters["threshold"] = std::to_string(threshold);
		}
	}
	// compare with anothe variable
	else if (node.imp == NodeImplementation::COMPARE_VAR) {
		char varBuffer[128];
		strncpy(varBuffer, node.parameters["variable"].c_str(), 127);
		varBuffer[127] = '\0';

		ImGui::Text("Variable Name:");
		if (ImGui::InputText("##variable", varBuffer, 128)) {
			node.parameters["variable"] = varBuffer;
		}

		std::string comparison = node.parameters.count("comparison") ?
			node.parameters["comparison"] : "greater";

		float threshold = node.parameters.count("threshold") ?
			std::stof(node.parameters["threshold"]) : 0.f;

		// dropdown
		ImGui::Text("Comparison:");
		if (ImGui::BeginCombo("##comparison", comparison.c_str())) {
			const char* comparisons[] = { "greater", "less", "equal" };
			const char* labels[] = { "Greater Than (>)", "Less Than (<)", "Equal To (=)" };

			for (int i = 0; i < 3; ++i) {
				bool isSelected = (comparison == comparisons[i]);

				if (ImGui::Selectable(labels[i], isSelected))
					node.parameters["comparison"] = comparisons[i];

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::DragFloat("Threshold Value", &threshold, 0.1f)) {
			node.parameters["threshold"] = std::to_string(threshold);
		}

		ImGui::Spacing();
		ImGui::TextDisabled("Compares with a variable");
		}
	/* ---------- END ---------- */
}

void VisualLogicEditor::renderSavePopup() {
	if (Editor::visualLogicState.showSavePopup) {
		ImGui::OpenPopup("Save Visual Logic");
		Editor::visualLogicState.showSavePopup = false;
	}

	static char filename[128] = "";

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Save Visual Logic", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter a filename:");
		ImGui::Spacing();

		// auto-fill with object name if empty
		if (filename[0] == '\0' && Editor::visualLogicState.targetObj) {
			std::string defaultName = Editor::visualLogicState.targetObj->getObjectName() + "_Logic";
			strncpy(filename, defaultName.c_str(), sizeof(filename) - 1);
		}

		ImGui::InputText("##filename", filename, sizeof(filename));
		ImGui::Spacing();
		ImGui::TextDisabled("(without .json extension)");
		ImGui::Spacing();

		// check if file already exists
		std::string fullPath = "/Logic/" + std::string(filename) + ".json";
		bool fileExists = std::filesystem::exists(fullPath);

		if (fileExists) {
			ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "Warning: File already exists!");
		}

		ImGui::Separator();

		// Save button
		if (ImGui::Button("Save", ImVec2(120, 0)) || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
			if (strlen(filename) > 0) {
				VisualLC* logic = Editor::visualLogicState.targetObj->getComponent<VisualLC>();
				if (logic) {
					std::string finalFilename = std::string(filename) + ".json";
					logic->filename = finalFilename;
					Editor::visualLogicState.currentGraphName = finalFilename;

					// Save the nodes
					logic->nodes = Editor::visualLogicState.nodes;
					logic->connections = Editor::visualLogicState.connections;

					std::string filepath = "/Logic/" + finalFilename;
					std::string err;
					if (JsonIO::saveVisualLogic(filepath,
						Editor::visualLogicState.nodes,
						Editor::visualLogicState.connections,
						&err)) {
						DebugLog::addMessage("Visual Logic saved as: " + finalFilename + "\n");
						DebugLog::addMessage("Saved " + std::to_string(Editor::visualLogicState.nodes.size()) + " nodes\n");
					}
					else {
						DebugLog::addMessage("Failed to save file: " + err);
					}

					// clear filename buffer
					filename[0] = '\0';

					ImGui::CloseCurrentPopup();
				}
			}
		}

		ImGui::SameLine();

		// Cancel button
		if (ImGui::Button("Cancel", ImVec2(120, 0)) || InputHandler::isKeyTriggered(GLFW_KEY_ESCAPE)) {
			filename[0] = '\0';
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void VisualLogicEditor::addNode(Vector2D pos, NodeType type, NodeImplementation imp) {
	LogicNode node;
	node.id = m_nextNodeId++;
	node.type = type;
	node.imp = imp;
	node.position = pos;
	node.hasInput = (type == NodeType::EVENT) ? false : true; // EVENT no input
	node.hasOutput = (type == NodeType::CONDITION) ? false : true; // CONDITION no output
	node.hasTrueOutput = (type == NodeType::CONDITION) ? true : false; // only for CONDITION
	node.hasFalseOutput = (type == NodeType::CONDITION) ? true : false; // only for CONDITION

	// ACTION nodes
	if(imp == NodeImplementation::PRINT_MESSAGE)
		node.parameters["message"] = "I love GAM";
	else if (imp == NodeImplementation::MOVE_OBJECT) {
		node.parameters["x"] = "0";
		node.parameters["y"] = "0";
	}
	else if (imp == NodeImplementation::ROTATE_OBJECT) {
		node.parameters["angle"] = "0"; // in degree 
	}
	else if (imp == NodeImplementation::SET_FLOAT) {
		node.parameters["variable"] = "shakeDir";
		node.parameters["value"] = "1";
	}
	// CONDITION nodes
	else if (imp == NodeImplementation::CHECK_KEY_HELD) {
		node.parameters["key"] = "SPACE";
	}
	else if (imp == NodeImplementation::COMPARE_VALUE) {
		node.parameters["property"] = "rotation";
		node.parameters["comparison"] = "greater";
		node.parameters["threshold"] = "0";
	}
	else if (imp == NodeImplementation::COMPARE_VAR) {
		node.parameters["variable"] = "shakeDir";
		node.parameters["comparison"] = "greater";
		node.parameters["threshold"] = "0";
	}

	Editor::visualLogicState.nodes.push_back(node);
}

ImU32 VisualLogicEditor::getNodeColor(NodeType type) {
	switch (type) {
	case NodeType::EVENT:
		return IM_COL32(76, 175, 80, 255);  // green
	case NodeType::ACTION:
		return IM_COL32(33, 150, 243, 255); // blue
	case NodeType::CONDITION:
		return IM_COL32(200, 150, 50, 255); // orange
	default:
		return IM_COL32(128, 128, 128, 255); // gray
	}
}

bool VisualLogicEditor::isPinHovered(ImVec2 pinPos, ImVec2 mousePos) {
	double distance = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));
	return distance < 10.f;
}

void VisualLogicEditor::createConnection(int fromNodeId, std::string fromPin, int toNodeId, std::string toPin) {
	// Check if connection already exists
	for (const LogicConnection& conn : Editor::visualLogicState.connections) {
		if (conn.fromNodeId == fromNodeId && conn.toNodeId == toNodeId) {
			return; // Connection already exists
		}
	}

	LogicConnection conn;
	conn.fromNodeId = fromNodeId;
	conn.toNodeId = toNodeId;
	conn.outputPin = fromPin;
	conn.inputPin = toPin;

	Editor::visualLogicState.connections.push_back(conn);

	DebugLog::addMessage("Created connection from node " + std::to_string(fromNodeId) +
		" to node " + std::to_string(toNodeId) + "\n");
}

void VisualLogicEditor::deleteSelectedNode() {
	if (m_selectedNodeId == -1) return;

	// remove connections
	Editor::visualLogicState.connections.erase(
		std::remove_if(Editor::visualLogicState.connections.begin(),
			Editor::visualLogicState.connections.end(),
			[this](const LogicConnection& conn) {
				return conn.fromNodeId == m_selectedNodeId ||
					conn.toNodeId == m_selectedNodeId;
			}),
		Editor::visualLogicState.connections.end()
	);

	// Remove node
	Editor::visualLogicState.nodes.erase(
		std::remove_if(Editor::visualLogicState.nodes.begin(),
			Editor::visualLogicState.nodes.end(),
			[this](const LogicNode& n) {
				return n.id == m_selectedNodeId;
			}),
		Editor::visualLogicState.nodes.end()
	);

	DebugLog::addMessage("Deleted node ID: " + std::to_string(m_selectedNodeId));
	m_selectedNodeId = -1;
}

void VisualLogicEditor::duplicateSelectedNode() {
	if (m_selectedNodeId == -1) return;

	// find selected node
	std::vector<LogicNode>::iterator it = std::find_if(Editor::visualLogicState.nodes.begin(),
		Editor::visualLogicState.nodes.end(),
		[this](const LogicNode& n) {
			return n.id == m_selectedNodeId;
		});

	if (it == Editor::visualLogicState.nodes.end()) return;

	// create duplicate
	LogicNode duplicate = *it;
	duplicate.id = m_nextNodeId++;
	duplicate.position.x += 50.f; // offset x
	duplicate.position.y += 50.f; // offset y

	Editor::visualLogicState.nodes.push_back(duplicate);

	// select the new node
	m_selectedNodeId = duplicate.id;

	DebugLog::addMessage("Duplicated node to ID: " + std::to_string(duplicate.id));
}

void VisualLogicEditor::selectNextNode(bool forward) {
	if (Editor::visualLogicState.nodes.empty()) return;

	if (m_selectedNodeId == -1) {
		// no selection, select first node
		m_selectedNodeId = Editor::visualLogicState.nodes[0].id;
		return;
	}

	// find current node index
	int currentIndex = -1;
	for (size_t i = 0; i < Editor::visualLogicState.nodes.size(); ++i) {
		if (Editor::visualLogicState.nodes[i].id == m_selectedNodeId) {
			currentIndex = static_cast<int>(i);
			break;
		}
	}

	if (currentIndex == -1) {
		m_selectedNodeId = Editor::visualLogicState.nodes[0].id;
		return;
	}

	// move to next/previous
	if (forward) {
		currentIndex = (currentIndex + 1) % Editor::visualLogicState.nodes.size();
	}
	else {
		currentIndex = (currentIndex - 1 + Editor::visualLogicState.nodes.size()) %
			Editor::visualLogicState.nodes.size();
	}

	m_selectedNodeId = Editor::visualLogicState.nodes[currentIndex].id;
}

ImVec2 VisualLogicEditor::getBezierPoint(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float t) {
	float u = 1.f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	ImVec2 p;
	p.x = (uuu * p0.x) + (3 * uu * t * p1.x) + (3 * u * tt * p2.x) + (ttt * p3.x);
	p.y = (uuu * p0.y) + (3 * uu * t * p1.y) + (3 * u * tt * p2.y) + (ttt * p3.y);

	return p;
}

bool VisualLogicEditor::isMouseNearBezier(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3,
	ImVec2 mousePos, float threshold) {
	
	// sample points along the curve and check distance
	const int samples = 20;
	for (int i = 0; i < samples; ++i) {
		float t = (float)i / (float)(samples - 1);
		ImVec2 point = getBezierPoint(p0, p1, p2, p3, t);

		float dx = mousePos.x - point.x;
		float dy = mousePos.y - point.y;
		float distance = sqrt(dx * dx + dy * dy);

		if (distance < threshold) {
			return true;
		}
	}
	return false;
}

void VisualLogicEditor::deleteSelectedConnection() {
	if (m_selectedConIndex == -1 ||
		m_selectedConIndex >= static_cast<int>(Editor::visualLogicState.connections.size())) {
		return;
	}

	const LogicConnection& conn = Editor::visualLogicState.connections[m_selectedConIndex];
	DebugLog::addMessage("Deleted connection from node " +
		std::to_string(conn.fromNodeId) + " to " +
		std::to_string(conn.toNodeId));

	Editor::visualLogicState.connections.erase(
		Editor::visualLogicState.connections.begin() + m_selectedConIndex);

	m_selectedConIndex = -1;
}

#endif