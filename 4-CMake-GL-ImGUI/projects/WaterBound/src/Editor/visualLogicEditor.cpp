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

	if (ImGui::BeginMenuBar()) {
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

		if (ImGui::BeginMenu("Add Node")) {
			if (ImGui::BeginMenu("Events")) {
				if (ImGui::MenuItem("On Start")) {
					LogicNode node;
					node.id = m_nextNodeId++;
					node.name = "On Start";
					node.type = NodeType::EVENT;
					node.position = ImVec2(100, 100);
					node.hasInput = false;
					node.hasOutput = true;
					Editor::visualLogicState.nodes.push_back(node);
				}

				if (ImGui::MenuItem("On Update")) {
					LogicNode node;
					node.id = m_nextNodeId++;
					node.name = "On Update";
					node.type = NodeType::EVENT;
					node.position = ImVec2(100, 150);
					node.hasInput = false;
					node.hasOutput = true;
					Editor::visualLogicState.nodes.push_back(node);
				}

				if (ImGui::MenuItem("On Collision (WIP)")) {
					LogicNode node;
					node.id = m_nextNodeId++;
					node.name = "On Collision";
					node.type = NodeType::EVENT;
					node.position = ImVec2(100, 200);
					node.hasInput = false;
					node.hasOutput = true;
					Editor::visualLogicState.nodes.push_back(node);
				}

				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Actions")) {
				if (ImGui::MenuItem("Print Message")) {
					LogicNode node;
					node.id = m_nextNodeId++;
					node.name = "Print Message";
					node.type = NodeType::ACTION;
					node.position = ImVec2(400, 100);
					node.hasInput = true;
					node.hasOutput = true;
					node.parameters["message"] = "I love GAM";
					Editor::visualLogicState.nodes.push_back(node);
				}

				if (ImGui::MenuItem("Move Object")) {
					LogicNode node;
					node.id = m_nextNodeId++;
					node.name = "Move Object";
					node.type = NodeType::ACTION;
					node.position = ImVec2(400, 200);
					node.hasInput = true;
					node.hasOutput = true;
					node.parameters["x"] = "0";
					node.parameters["y"] = "0";
					Editor::visualLogicState.nodes.push_back(node);
				}

				ImGui::EndMenu();
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
	for (auto& node : Editor::visualLogicState.nodes) {
		renderNode(drawList, node, canvasPos, isCanvasHovered);
	}

	// draw line when creating connection
	if (m_isConnecting) {
		ImVec2 mousePos = ImGui::GetMousePos();
		drawList->AddLine(m_connectionStartPos, mousePos, IM_COL32(255, 255, 0, 255), 3.f);
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

	drawList->PopClipRect();

	// Info panel
	if (Editor::visualLogicState.targetObj) {
		ImGui::Text("Selected Object: %s", Editor::visualLogicState.targetObj->getObjectName().c_str());
	}
	ImGui::Text("Nodes: %d", (int)Editor::visualLogicState.nodes.size());

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

// In VisualLogicEditor class
void VisualLogicEditor::clear() {
	Editor::visualLogicState.nodes.clear();
	Editor::visualLogicState.connections.clear();
	Editor::visualLogicState.targetObj = nullptr;
	m_selectedNodeId = -1;
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
		IM_COL32(255, 255, 255, 255), node.name.c_str());
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


		// delete
		if (deleteHovered && InputHandler::isMouseLeftClicked()) {
			// remove connection
			Editor::visualLogicState.connections.erase(
				std::remove_if(Editor::visualLogicState.connections.begin(), Editor::visualLogicState.connections.end(),
					[&node](const LogicConnection& conn) {
						return conn.fromNodeId == node.id || conn.toNodeId == node.id;
					}),
				Editor::visualLogicState.connections.end()
			);

			// remove node
			Editor::visualLogicState.nodes.erase(std::remove_if(Editor::visualLogicState.nodes.begin(), Editor::visualLogicState.nodes.end(),
				[&node](const LogicNode& n) { return n.id == node.id; }),
				Editor::visualLogicState.nodes.end());

			if (m_selectedNodeId == node.id) {
				m_selectedNodeId = -1;
			}
		}
	}
	/* ---------- END ---------- */
}

void VisualLogicEditor::renderConnections(ImDrawList* drawList, ImVec2 canvasPos) {
	for (const auto& conn : Editor::visualLogicState.connections) {
		LogicNode* fromNode = nullptr;
		LogicNode* toNode = nullptr;

		for (auto& node : Editor::visualLogicState.nodes) {
			if (node.id == conn.fromNodeId) fromNode = &node;
			if (node.id == conn.toNodeId) toNode = &node;
		}

		if (!fromNode || !toNode) continue;

		ImVec2 fromPos = ImVec2(canvasPos.x + fromNode->position.x + m_canvasOffset.x + 250.f, canvasPos.y + fromNode->position.y + m_canvasOffset.y + 40.f);

		if (conn.outputPin == "True") fromPos.y -= 10.f;
		if (conn.outputPin == "False") fromPos.y += 10.f;

		ImVec2 toPos = ImVec2(canvasPos.x + toNode->position.x + m_canvasOffset.x, canvasPos.y + toNode->position.y + m_canvasOffset.y + 40.f);

		// draw bezier curve
		ImVec2 cp1(fromPos.x + 50, fromPos.y);
		ImVec2 cp2(toPos.x - 50, toPos.y);

		ImU32 lineColor = IM_COL32(200, 200, 200, 255);
		if (conn.outputPin == "True") lineColor = IM_COL32(100, 255, 100, 255);
		if (conn.outputPin == "False") lineColor = IM_COL32(255, 100, 100, 255);

		drawList->AddBezierCubic(fromPos, cp1, cp2, toPos, lineColor, 2.0f);
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

	ImGui::Text("Node: %s", node.name.c_str());
	ImGui::Separator();

	if (node.name == "Print Message") {
		char buffer[256];
		strncpy(buffer, node.parameters["message"].c_str(), 255);

		if (ImGui::InputText("Message", buffer, 256)) {
			node.parameters["message"] = buffer;
		}
	}
	else  if (node.name == "Move Object") {
		float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.0f;
		float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.0f;

		if (ImGui::DragFloat("X Offset", &x, 0.1f)) {
			node.parameters["x"] = std::to_string(x);
		}
		if (ImGui::DragFloat("Y Offset", &y, 0.1f)) {
			node.parameters["y"] = std::to_string(y);
		}
	}
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
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning: File already exists!");
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

//void VisualLogicEditor::addEventNode(ImVec2 position) {
//	LogicNode node;
//	node.id = m_nextNodeId++;
//	node.name = "On Start";
//	node.type = NodeType::EVENT;
//	node.position = position;
//	node.hasInput = false;
//	node.hasOutput = true;
//
//	m_nodes.push_back(node);
//
//}
//
//void VisualLogicEditor::addActionNode(ImVec2 position) {
//	LogicNode node;
//	node.id = m_nextNodeId++;
//	node.name = "Print Message";
//	node.type = NodeType::ACTION;
//	node.position = position;
//	node.hasInput = true;
//	node.hasOutput = true;
//
//	m_nodes.push_back(node);
//}

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
	float distance = sqrt(pow(mousePos.x - pinPos.x, 2) + pow(mousePos.y - pinPos.y, 2));
	return distance < 10.0f;
}

void VisualLogicEditor::createConnection(int fromNodeId, std::string fromPin, int toNodeId, std::string toPin) {
	// Check if connection already exists
	for (const auto& conn : Editor::visualLogicState.connections) {
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

#endif