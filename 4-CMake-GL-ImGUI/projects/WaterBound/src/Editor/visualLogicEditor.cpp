/* Start Header ************************************************************************/
/*!
\file       visualLogicEditor.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       January, 15th, 2026
\brief      A visual logic editor for creating, connecting, and managing event, condition, and action				nodes with drag-and-drop, zoom, pan, and property editing. Supports saving/loading logic,			node duplication/deletion, and interactive connections for game object behavior scripting

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
	if (!Editor::visualLogicState.showEditor) {
		if (!Editor::visualLogicState.currentGraphName.empty()) clear();
		return;
	}

	ImGui::Begin("Visual Logic Editor", &Editor::visualLogicState.showEditor, ImGuiWindowFlags_MenuBar);

	Editor::visualLogicState.isFocus = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::BeginMenuBar()) {
		/* -------- File -------- */
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				saveLogic();
			}

			if (ImGui::MenuItem("Save and Close")) {
				Editor::visualLogicState.wantCloseEditor = true;
				saveLogic();
			}

			ImGui::EndMenu();
		}
		/* -------- END -------- */


		/* -------- View -------- */
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Node Palette", nullptr, &m_showNodePalette);

			if (ImGui::MenuItem("Reset View")) {
				m_canvasOffset = ImVec2(50, 50);
				m_zoom = 1.f;
			}

			ImGui::EndMenu();
		}
		/* -------- END -------- */

		ImGui::EndMenuBar();
	}


	/* ------ Node Palette ------ */
	if (m_showNodePalette) {
		ImGui::BeginChild("Node Palette", ImVec2(Editor::visualLogicState.nodePaletteWidth, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
		renderNodePalette();
		ImGui::EndChild();

		verticalSplitter(&Editor::visualLogicState.nodePaletteWidth);
		ImGui::SameLine();
	}
	/* ------ END ------ */


	//float canvasWidth = m_showNodePalette ? ImGui::GetContentRegionAvail().x * 0.7f : ImGui::GetContentRegionAvail().x * 0.75f;


	/* ------ Canvas ------ */
	ImGui::BeginChild("Canvas", ImVec2(-Editor::visualLogicState.propertiesWidth - 8, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

	if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_RIGHT) && ImGui::IsWindowHovered() && !m_isDragging && !m_isConnecting) {
		ImVec2 mouseScreenPos = ImGui::GetMousePos();
		ImVec2 canvasPos = ImGui::GetCursorScreenPos();

		m_menuSpawnPos.x = (mouseScreenPos.x - canvasPos.x - m_canvasOffset.x) / m_zoom;
		m_menuSpawnPos.y = (mouseScreenPos.y - canvasPos.y - m_canvasOffset.y) / m_zoom;

		m_showNodeMenu = true;
		m_nodeSearchBuffer[0] = '\0';
		ImGui::OpenPopup("AddNodeContextMenu");
	}

	// render context menu
	if (ImGui::BeginPopup("AddNodeContextMenu")) {
		renderNodeSearchMenu();
		ImGui::EndPopup();
	}
	else {
		m_showNodeMenu = false;
	}

	// canvas area
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();

	ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
	bool isCanvasHovered = ImGui::IsItemHovered();

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

	// grid background
	const float GRIDSIZE = Editor::VLC::GRID_SIZE * m_zoom;
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
			ImVec2 nodePos = ImVec2(canvasPos.x + (node.position.x * m_zoom) + m_canvasOffset.x,
				canvasPos.y + (node.position.y * m_zoom) + m_canvasOffset.y);
			ImVec2 nodeSize(Editor::VLC::NODE_WIDTH * m_zoom, Editor::VLC::NODE_HEIGHT * m_zoom);

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

				ImVec2 fromPos = ImVec2(canvasPos.x + (fromNode->position.x * m_zoom) + m_canvasOffset.x + (250.f * m_zoom),
					canvasPos.y + (fromNode->position.y * m_zoom) + m_canvasOffset.y + (40.f * m_zoom));

				if (conn.outputPin == "True") fromPos.y -= (Editor::VLC::PIN_OFFSET_Y * m_zoom);
				if (conn.outputPin == "False") fromPos.y += (Editor::VLC::PIN_OFFSET_Y * m_zoom);

				ImVec2 toPos = ImVec2(canvasPos.x + (toNode->position.x * m_zoom) + m_canvasOffset.x,
					canvasPos.y + (toNode->position.y * m_zoom) + m_canvasOffset.y + (40.f * m_zoom));

				ImVec2 cp1(fromPos.x + (Editor::VLC::CONNECTION_BEZIER_OFFSET * m_zoom), fromPos.y);
				ImVec2 cp2(toPos.x - (Editor::VLC::CONNECTION_BEZIER_OFFSET * m_zoom), toPos.y);

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

	// zoom in/out
	if (isCanvasHovered && InputHandler::getMouseScroll() != 0.f) {
		float scroll = InputHandler::getMouseScroll();
		m_zoom += scroll * 0.1f;
		m_zoom = glm::clamp(m_zoom, 0.5f, 2.f);
	}

	// shortcut
	if (Editor::visualLogicState.isFocus && !ImGui::GetIO().WantTextInput) {
		// delete node/con with DEL
		if ((InputHandler::isKeyTriggered(GLFW_KEY_DELETE))) {
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

	ImGui::EndChild();
	/* ------ END ------ */


	ImGui::SameLine();

	/* ------ Properties Panel ------ */
	verticalSplitter(&Editor::visualLogicState.propertiesWidth, true);

	ImGui::BeginChild("Properties", ImVec2(Editor::visualLogicState.propertiesWidth, 0), true);
	renderNodeProperties();
	ImGui::EndChild();
	/* ------ END ------ */


	// Info panel
	if (Editor::visualLogicState.targetObj) {
		ImGui::Text("Selected Object: %s", Editor::visualLogicState.targetObj->getObjectName().c_str());
	}
	ImGui::Text("Nodes: %d", (int)Editor::visualLogicState.nodes.size());


	ImGui::End();


	// popup
	renderSavePopup();
	//renderNodePalette();
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
	Editor::visualLogicState.currentGraphName.clear();
	Editor::visualLogicState.isFocus = false;
	Editor::visualLogicState.wantCloseEditor = false;
	Editor::visualLogicState.showSavePopup = false;
	Editor::visualLogicState.targetObj = nullptr;

	m_selectedNodeId = -1;
	m_selectedConIndex = -1;

	m_connectingFromPin.clear(); 
	m_connectionStartPos = ImVec2(0.0f, 0.0f);

	m_isDragging = false;
	m_draggingNodeId = -1;
	m_isConnecting = false;
	m_connectingFromNodeId = -1;
	m_nextNodeId = 1;
}

void VisualLogicEditor::saveLogic() {
	// if open from asset browser
	if (!Editor::visualLogicState.targetObj) {
		if (Editor::visualLogicState.currentGraphName.empty()) {
			DebugLog::addMessage("Cannot save: no target object for unnamed script.");
			return;
		}

		std::string filename = "/Logic/" + Editor::visualLogicState.currentGraphName;
		std::string err;

		if (JsonIO::saveVisualLogic(filename, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {

			reloadAllVisualLogic(filename);

			if (Editor::visualLogicState.wantCloseEditor) {
				Editor::visualLogicState.showEditor = false;
				Editor::visualLogicState.isFocus = false;
				Editor::visualLogicState.wantCloseEditor = false;
				Editor::visualLogicState.currentGraphName.clear();
				m_selectedNodeId = -1;
			}

			DebugLog::addMessage("Saved Visual Logic: " + filename);
		}
		else {
			DebugLog::addMessage("Failed to save: " + err);
		}

		return;
	}

	// open from inspector
	VisualLC* logic = Editor::visualLogicState.targetObj->getComponent<VisualLC>();
	if (!logic) return;

	if (logic->filename.empty()) {
		Editor::visualLogicState.showSavePopup = true;
		return;
	}

	std::string filepath = "/Logic/" + logic->filename;

	std::string err;
	if (JsonIO::saveVisualLogic(filepath, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {
		reloadAllVisualLogic(logic->filename);

		if (Editor::visualLogicState.wantCloseEditor) {
			Editor::visualLogicState.showEditor = false;
			Editor::visualLogicState.isFocus = false;
			Editor::visualLogicState.wantCloseEditor = false;
			m_selectedNodeId = -1;
		}

		// reset hover check cache
		if (Editor::visualLogicState.targetObj) {
			if (logic) {
				logic->hoverNodesChecked = false;
			}
		}

		DebugLog::addMessage("Saved Visual Logic: " + filepath);
	}
	else {
		DebugLog::addMessage("Failed to save: " + err);
	}
}

void VisualLogicEditor::loadLogic(const std::string& filename) {
	std::string filepath = "/Logic/" + filename;
	std::string err;

	// if open from asset browser
	if (!Editor::visualLogicState.targetObj) {
		clear();
		if (JsonIO::loadVisualLogic(filepath, Editor::visualLogicState.nodes, Editor::visualLogicState.connections, &err)) {
			updateNextNodeId();

			Editor::visualLogicState.currentGraphName = filename;
			Editor::visualLogicState.showEditor = true;

			DebugLog::addMessage("Loaded Visual Logic: " + filename);
		}
		else {
			DebugLog::addMessage("Failed to load: " + err);
		}
		return;
	}

	// if open from inspector
	VisualLC* logic = Editor::visualLogicState.targetObj->getComponent<VisualLC>();
	if (!logic) {
		DebugLog::addMessage("ERROR: Target object has no VisualLC component");
		return;
	}

	clear();
	if (JsonIO::loadVisualLogic(filepath, logic->nodes, logic->connections, &err)) {
		logic->filename = filename;

		// sync to editor state for editing
		Editor::visualLogicState.nodes = logic->nodes;
		Editor::visualLogicState.connections = logic->connections;
		updateNextNodeId();

		Editor::visualLogicState.currentGraphName = filename;
		Editor::visualLogicState.showEditor = true;

		DebugLog::addMessage("Loaded Visual Logic Graph: " + filename);
	}
	else {
		DebugLog::addMessage("Failed to load: " + err);
	}
}

void VisualLogicEditor::renderNode(ImDrawList* drawList, LogicNode& node, ImVec2 canvasPos, bool isCanvasHovered) {
	// calc screen position
	ImVec2 nodePos = ImVec2(canvasPos.x + (node.position.x * m_zoom) + m_canvasOffset.x,
		canvasPos.y + (node.position.y * m_zoom) + m_canvasOffset.y);

	ImVec2 nodeSize(250.f * m_zoom, 80.f * m_zoom);


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
		node.position.x = (mousePos.x - canvasPos.x - m_canvasOffset.x - m_dragOffset.x) / m_zoom;
		node.position.y = (mousePos.y - canvasPos.y - m_canvasOffset.y - m_dragOffset.y) / m_zoom;
	}
	/* ---------- END ---------- */


	/* ---------- Draw Nodes ---------- */
	ImU32 nodeColor = getNodeColor(node.type);
	ImU32 borderColor = (m_selectedNodeId == node.id) ?
		IM_COL32(255, 255, 0, 255) : IM_COL32(100, 100, 100, 255);
	float borderThickness = (m_selectedNodeId == node.id) ? Editor::VLC::BORDER_THICK : Editor::VLC::BORDER_THIN;
	borderThickness *= m_zoom;

	drawList->AddRectFilled(nodePos,
		ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y),
		nodeColor, Editor::VLC::BORDER_RADIUS * m_zoom);

	drawList->AddRect(nodePos,
		ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y),
		borderColor, 4.f * m_zoom, 0, borderThickness);

	// node name
	drawList->AddText(ImVec2(nodePos.x + (Editor::VLC::NODE_PADDING * m_zoom), nodePos.y + (Editor::VLC::NODE_PADDING * m_zoom)),
		IM_COL32(255, 255, 255, 255), JsonIO::nodeImplToString(node.imp).c_str());
	/* ---------- END ---------- */


	/* ---------- Draw Input Pin ---------- */
	if (node.hasInput) {
		ImVec2 pinPos(nodePos.x, nodePos.y + (Editor::VLC::PIN_Y_OFFSET * m_zoom));
		drawList->AddCircleFilled(pinPos, Editor::VLC::PIN_RADIUS * m_zoom, IM_COL32(200, 200, 200, 255));
		
		drawList->AddText(ImVec2(pinPos.x + (Editor::VLC::PIN_TEXT_OFFSET * m_zoom), pinPos.y - (Editor::VLC::PIN_TEXT_Y_OFFSET * m_zoom)),
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
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + (Editor::VLC::PIN_Y_OFFSET * m_zoom));
		drawList->AddCircleFilled(pinPos, Editor::VLC::PIN_RADIUS * m_zoom, IM_COL32(200, 200, 200, 255));

		ImVec2 textSize = ImGui::CalcTextSize("Out");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - (12 * m_zoom), pinPos.y - (8 * m_zoom)),
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
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + (Editor::VLC::PIN_TRUE_Y * m_zoom));
		drawList->AddCircleFilled(pinPos, 6.f * m_zoom, IM_COL32(100, 255, 100, 255));

		ImVec2 textSize = ImGui::CalcTextSize("True");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - (12 * m_zoom), pinPos.y - (8 * m_zoom)),
			IM_COL32(255, 255, 255, 255), "True");

		if (!m_isConnecting && isPinHovered(pinPos, mousePos) && InputHandler::isMouseLeftClicked()) {
			m_isConnecting = true;
			m_connectingFromNodeId = node.id;
			m_connectingFromPin = "True";
			m_connectionStartPos = pinPos;
		}
	}

	if (node.hasFalseOutput) {
		ImVec2 pinPos(nodePos.x + nodeSize.x, nodePos.y + (Editor::VLC::PIN_FALSE_Y * m_zoom));
		drawList->AddCircleFilled(pinPos, (6.f * m_zoom), IM_COL32(255, 100, 100, 255));

		ImVec2 textSize = ImGui::CalcTextSize("False");
		drawList->AddText(ImVec2(pinPos.x - textSize.x - (12 * m_zoom), pinPos.y - (8 * m_zoom)),
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
		ImVec2 deletePos(nodePos.x + nodeSize.x - (Editor::VLC::DELETE_OFFSET_X * m_zoom), nodePos.y + (Editor::VLC::DELETE_OFFSET_Y * m_zoom));
		ImVec2 deleteSize(Editor::VLC::DELETE_SIZE * m_zoom, Editor::VLC::DELETE_SIZE * m_zoom);

		bool deleteHovered = mousePos.x >= deletePos.x && mousePos.x <= deletePos.x + deleteSize.x &&
			mousePos.y >= deletePos.y && mousePos.y <= deletePos.y + deleteSize.y;

		ImU32 deleteColor = deleteHovered ? IM_COL32(255, 80, 80, 255) : IM_COL32(200, 60, 60, 255);

		drawList->AddRectFilled(deletePos,
			ImVec2(deletePos.x + deleteSize.x, deletePos.y + deleteSize.y),
			deleteColor, Editor::VLC::BORDER_RADIUS * m_zoom);

		drawList->AddText(ImVec2(deletePos.x + (Editor::VLC::DELETE_TEXT_OFFSET_X * m_zoom), deletePos.y + (Editor::VLC::DELETE_OFFSET_Y * m_zoom)),
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

		ImVec2 fromPos = ImVec2(canvasPos.x + (fromNode->position.x * m_zoom) + m_canvasOffset.x + (250.f * m_zoom), canvasPos.y + (fromNode->position.y * m_zoom) + m_canvasOffset.y + (40.f * m_zoom));

		if (conn.outputPin == "True") fromPos.y -= (10.f * m_zoom);
		if (conn.outputPin == "False") fromPos.y += (10.f * m_zoom);

		ImVec2 toPos = ImVec2(canvasPos.x + (toNode->position.x * m_zoom) + m_canvasOffset.x, canvasPos.y + (toNode->position.y * m_zoom) + m_canvasOffset.y + (40.f * m_zoom));

		// draw curve
		ImVec2 cp1(fromPos.x + (50 * m_zoom), fromPos.y);
		ImVec2 cp2(toPos.x - (50 * m_zoom), toPos.y);

		ImU32 lineColor = IM_COL32(200, 200, 200, 255);
		if (conn.outputPin == "True") lineColor = IM_COL32(100, 255, 100, 255);
		if (conn.outputPin == "False") lineColor = IM_COL32(255, 100, 100, 255);

		// if mouse is near curve
		bool isHovered = isMouseNearBezier(fromPos, cp1, cp2, toPos, mousePos, 10.f * m_zoom);

		// if connection selected
		bool isSelected = (m_selectedConIndex >= 0 &&
			m_selectedConIndex == static_cast<int>(i));

		float lineWidth = Editor::VLC::LINE_NORMAL * m_zoom;
		if (isSelected) {
			lineColor = IM_COL32(255, 255, 0, 255); // yellow when selected
			lineWidth = Editor::VLC::LINE_SELECTED * m_zoom;
		}
		else if (isHovered) {
			lineWidth = Editor::VLC::LINE_HOVERED * m_zoom; // slightly thicker when hovered
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


	/* ---------- EVENT ---------- */
	if (node.type == NodeType::EVENT) {
		if (node.imp == NodeImplementation::ON_COLLISION_ENTER || node.imp == NodeImplementation::WHILE_COLLIDING || node.imp == NodeImplementation::ON_COLLISION_EXIT) {
			ImGui::Text("Collision Filter");
			ImGui::Spacing();

			std::string filterType = node.parameters.count("colFilterType") ?
				node.parameters["colFilterType"] : "any";

			if (ImGui::BeginCombo("Collide with", filterType.c_str())) {
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
				}

				ImGui::EndCombo();
			}

			if (filterType != "any") {
				std::string currentValue = node.parameters.count("colFilterValue") ? node.parameters["colFilterValue"] : "";
				char valueBuffer[128];
				strncpy(valueBuffer, node.parameters["colFilterValue"].c_str(), 127);
				valueBuffer[127] = '\0';

				if (filterType == "name") {
					ImGui::Text("Object Name:");

					if (ImGui::InputText("##colFilterValue", valueBuffer, 128)) {
						node.parameters["colFilterValue"] = valueBuffer;
					}

					if (ImGui::BeginCombo("##quickSelect", "Quick Select")) {
						auto& cached = CachedGameObjects::Instance().getCachedObjects();
						for (GameObject* obj : cached) {
							if (ImGui::Selectable(obj->getObjectName().c_str())) {
								node.parameters["colFilterValue"] = obj->getObjectName();
							}
						}
						ImGui::EndCombo();
					}
				}
				else if (filterType == "tag") {
					ImGui::Text("Collision Tag:");

					if (ImGui::InputText("##colFilterValue", valueBuffer, 128)) {
						node.parameters["colFilterValue"] = valueBuffer;
					}

					if (ImGui::BeginCombo("##quickSelectTag", "Quick Select")) {
						std::vector<std::string> uniqueTags = GlobalRegistry::Instance().getCollisionTags();

						for (const std::string& tag : uniqueTags) {
							if (ImGui::Selectable(tag.c_str())) {
								node.parameters["colFilterValue"] = tag;
							}
						}

						ImGui::EndCombo();
					}
				}
			}
		}
		else if (node.imp == NodeImplementation::ON_BULLET_SHOOT) {
			ImGui::TextWrapped("This event triggers once bullet is fired.");
		}
		else if (node.imp == NodeImplementation::ON_BULLET_REFILL) {
			ImGui::TextWrapped("This event triggers once bullet is refilled.");
		}
	}
	/* ---------- ACTION ---------- */
	else if (node.type == NodeType::ACTION) {
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

			if (ImGui::DragFloat("X Speed (units/sec)", &x, 0.1f)) {
				node.parameters["x"] = std::to_string(x);
			}
			if (ImGui::DragFloat("Y Speed (units/sec)", &y, 0.1f)) {
				node.parameters["y"] = std::to_string(y);
			}
		}
		// rotate obj
		else if (node.imp == NodeImplementation::ROTATE_OBJECT) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;

			if (ImGui::DragFloat("Angle (degrees/sec)", &angle, 1.f)) {
				node.parameters["angle"] = std::to_string(angle);

				ImGui::Spacing();
				ImGui::TextDisabled("Positive = counter-clockwise");
			}
		}
		// set obj rotation to x degree
		else if (node.imp == NodeImplementation::SET_ROTATION) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;

			if (ImGui::DragFloat("Rotation (degrees)", &angle, 1.f)) {
				node.parameters["angle"] = std::to_string(angle);
			}
		}
		// set obj position to x,y
		else if (node.imp == NodeImplementation::SET_POSITION) {
			ImGui::Text("Set Position");
			ImGui::Separator();

			static char xBuffer[64];
			static char yBuffer[64];

			std::string currentX = node.parameters.count("x") ?
				node.parameters["x"] : "0";
			std::string currentY = node.parameters.count("y") ?
				node.parameters["y"] : "0";

			strncpy_s(xBuffer, currentX.c_str(), sizeof(xBuffer) - 1);
			strncpy_s(yBuffer, currentY.c_str(), sizeof(yBuffer) - 1);

			ImGui::Text("X Position:");
			if (ImGui::InputText("##x", xBuffer, sizeof(xBuffer))) {
				node.parameters["x"] = xBuffer;
			}

			ImGui::Text("Y Position:");
			if (ImGui::InputText("##y", yBuffer, sizeof(yBuffer))) {
				node.parameters["y"] = yBuffer;
			}

			ImGui::TextWrapped("Sets the object's position to the specified coordinates.");
		}
		// set obj scale to x,y
		else if (node.imp == NodeImplementation::SET_SCALE) {
			ImGui::Text("Set Scale");
			ImGui::Separator();

			static char scaleXBuffer[64];
			static char scaleYBuffer[64];

			std::string currentScaleX = node.parameters.count("scaleX") ?
				node.parameters["scaleX"] : "1";
			std::string currentScaleY = node.parameters.count("scaleY") ?
				node.parameters["scaleY"] : "1";

			strncpy_s(scaleXBuffer, currentScaleX.c_str(), sizeof(scaleXBuffer) - 1);
			strncpy_s(scaleYBuffer, currentScaleY.c_str(), sizeof(scaleYBuffer) - 1);

			ImGui::Text("Scale X:");
			if (ImGui::InputText("##scaleX", scaleXBuffer, sizeof(scaleXBuffer))) {
				node.parameters["scaleX"] = scaleXBuffer;
			}

			ImGui::Text("Scale Y:");
			if (ImGui::InputText("##scaleY", scaleYBuffer, sizeof(scaleYBuffer))) {
				node.parameters["scaleY"] = scaleYBuffer;
			}
		}
		// scale obj proportionally
		else if (node.imp == NodeImplementation::SCALE_PROPORTIONAL) {
			ImGui::Text("Scale Proportional");

			float scale = node.parameters.count("scale") ? std::stof(node.parameters["scale"]) : 2.f;

			if (ImGui::DragFloat("Scale Factor", &scale, 0.1f)) {
				node.parameters["scale"] = std::to_string(scale);
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
		else if (node.imp == NodeImplementation::MODIFY_FLOAT) {
			char varBuffer[128];
			strncpy(varBuffer, node.parameters["variable"].c_str(), 127);
			varBuffer[127] = '\0';

			if (ImGui::InputText("Variable Name", varBuffer, 128)) {
				node.parameters["variable"] = varBuffer;
			}

			ImGui::Text("Modify By (negative to subtract)");

			float value = node.parameters.count("value") ?
				std::stof(node.parameters["value"]) : 0.f;

			if (ImGui::DragFloat("##value", &value, 0.1f)) {
				node.parameters["value"] = std::to_string(value);
			}

			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Use -1 to subtract, +1 to add");
		}
		// play Sound
		else if (node.imp == NodeImplementation::PLAY_SOUND) {
			AudioComponent* audio = nullptr;
			if (Editor::visualLogicState.targetObj) {
				audio = Editor::visualLogicState.targetObj->getComponent<AudioComponent>();
			}

			ImGui::Text("Audio Channel:");

			if (audio && !audio->audioChannels.empty()) {
				std::string currentChannel = (node.parameters.count("channel")) ?
					node.parameters["channel"] : "(none)";

				// in case audio is using the default channel (audio component thingy)
				if (!audio->getChannel(currentChannel)) {
					currentChannel = "(none)";
				}

				// dropdown of channels
				if (ImGui::BeginCombo("##channel", currentChannel.c_str())) {
					for (const auto& [channelName, ch] : audio->audioChannels) {
						bool isSelected = (currentChannel == channelName);

						std::string label = channelName;
						if (!ch.audioFile.empty()) {
							std::string fileName = std::filesystem::path(ch.audioFile).filename().string();
							label += " (" + fileName + ")";
						}

						if (ImGui::Selectable(label.c_str(), isSelected)) {
							node.parameters["channel"] = channelName;
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Spacing();

				// channel not selected then ask to select one
				if (currentChannel == "(none)" || !node.parameters.count("channel")) {
					ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Select a channel to play sound");
				}
				// channel selected then show the info
				else if (audio->hasChannel(currentChannel)) {
					const AudioChannel* ch = audio->getChannel(currentChannel);
					if (ch && !ch->audioFile.empty()) {
						ImGui::TextDisabled("Will play: %s",
							std::filesystem::path(ch->audioFile).filename().string().c_str());
						ImGui::TextDisabled("Loop: %s | Volume: %.2f",
							ch->loop ? "Yes" : "No", ch->volume);
					}
					else {
						ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
							"Warning: Channel has no audio file!");
					}
				}
			}
			else {
				// no audio component or no channels
				ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f),
					"No audio channels available!");
				ImGui::TextDisabled("Add channels in the Audio Component first");
			}
		}
		else if (node.imp == NodeImplementation::SET_STATE) {
			/*StateMachine* sm = nullptr;
			if (Editor::visualLogicState.targetObj) {
				sm = Editor::visualLogicState.targetObj->getComponent<StateMachine>();
			}*/

			ImGui::Text("State Machine:");

			/*if (sm) {*/
			std::string currentState = node.parameters.count("state") ?
				node.parameters["state"] : JsonIO::stateNames[0];

			if (ImGui::BeginCombo("##state", currentState.c_str())) {
				const std::vector<std::string>& stateNames = JsonIO::stateNames;

				for (const std::string& stateName : stateNames) {
					bool isSelected = (currentState == stateName);

					if (ImGui::Selectable(stateName.c_str(), isSelected)) {
						node.parameters["state"] = stateName;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::TextDisabled("Remember to add StateMachine to object.");

			/*}
			else {
				ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Add Animation Component first");
				ImGui::TextDisabled("Add a State Machine component first");
			}*/
		}
		else if (node.imp == NodeImplementation::RESET_TIMER) {
			ImGui::Text("Reset Timer");
			ImGui::Separator();

			std::vector<std::string> availableTimers;
			for (const LogicNode& node1 : Editor::visualLogicState.nodes) {
				if ((node1.imp == NodeImplementation::CHECK_TIMER ||
					node1.imp == NodeImplementation::RESET_TIMER) &&
					node1.parameters.count("timerName")) {

					std::string timer = node1.parameters.at("timerName");

					if (std::find(availableTimers.begin(), availableTimers.end(), timer) == availableTimers.end()) {
						availableTimers.push_back(timer);

					}
				}
			}

			std::string currentTimer = node.parameters.count("timerName") ?
				node.parameters["timerName"] : "timer_0";

			ImGui::Text("Timer ID:");
			if (ImGui::BeginCombo("##timerName", currentTimer.c_str())) {
				for (const std::string& timer : availableTimers) {
					bool isSelected = (currentTimer == timer);
					if (ImGui::Selectable(timer.c_str(), isSelected)) {
						node.parameters["timerName"] = timer;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();
			ImGui::TextWrapped("Resets the selected timer to 0 seconds.");
		}
		else if (node.imp == NodeImplementation::SET_TEXTURE) {
			ImGui::Text("Set Texture");
			std::string texturePath = node.parameters.count("textureFile") ?
				node.parameters["textureFile"] : "";

			char pathBuffer[256];
			strncpy(pathBuffer, texturePath.c_str(), 255);
			pathBuffer[255] = '\0';

			if (ImGui::InputText("##textureFile", pathBuffer, 256)) {
				node.parameters["textureFile"] = pathBuffer;
			}

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
					const char* droppedPath = (const char*)payload->Data;
					node.parameters["textureFile"] = droppedPath;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Spacing();
			if (!texturePath.empty()) {
				ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
					"Current: %s", texturePath.c_str());
			}
			else {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f),
					"No texture set");
			}

			ImGui::Spacing();
			ImGui::TextWrapped("Drag and drop a texture from the asset browser");
		}
		else  if (node.imp == NodeImplementation::SET_POSITION_ON_COLLIDED) {
			ImGui::Text("Set Position on Object Collided:");

			float deltaXCollided = node.parameters.count("deltaXCollided") ? std::stof(node.parameters["deltaXCollided"]) : 0.f;
			float deltaYCollided = node.parameters.count("deltaYCollided") ? std::stof(node.parameters["deltaYCollided"]) : 0.f;

			if (ImGui::DragFloat("Delta X", &deltaXCollided, 0.1f)) {
				node.parameters["deltaXCollided"] = std::to_string(deltaXCollided);
			}
			if (ImGui::DragFloat("Delta Y", &deltaYCollided, 0.1f)) {
				node.parameters["deltaYCollided"] = std::to_string(deltaYCollided);
			}

			ImGui::TextDisabled("P/s: can't change position of object that has input component");
		}
		else if (node.imp == NodeImplementation::SET_STATE_ON_COLLIDED) {

			ImGui::Text("Set State on Object Collided:");

			std::string currentState = node.parameters.count("targetStateCollided") ?
				node.parameters["targetStateCollided"] : JsonIO::stateNames[0];

			if (ImGui::BeginCombo("##targetStateCollided", currentState.c_str())) {
				const std::vector<std::string>& stateNames = JsonIO::stateNames;

				for (const std::string& stateName : stateNames) {
					bool isSelected = (currentState == stateName);

					if (ImGui::Selectable(stateName.c_str(), isSelected)) {
						node.parameters["targetStateCollided"] = stateName;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::TextDisabled("Remember to add Animation to Collided Object");
		}
		else if (node.imp == NodeImplementation::APPLY_FORCE_ON_COLLIDED) {

			ImGui::Text("Apply Force on Object Collided:");

			float forceX = node.parameters.count("forceX") ? std::stof(node.parameters["forceX"]) : 0.f;
			float forceY = node.parameters.count("forceY") ? std::stof(node.parameters["forceY"]) : 0.f;

			if (ImGui::DragFloat("Force X", &forceX, 0.1f)) {
				node.parameters["forceX"] = std::to_string(forceX);
			}
			if (ImGui::DragFloat("Force Y", &forceY, 0.1f)) {
				node.parameters["forceY"] = std::to_string(forceY);
			}
		}
		else if (node.imp == NodeImplementation::SET_GLOBAL_FLOAT) {
			ImGui::Text("Set Global Float Variable");

			char varBuffer[128];
			std::string currentVar = node.parameters.count("variable") ?
				node.parameters["variable"] : "globalVar";
			strncpy(varBuffer, currentVar.c_str(), 127);
			varBuffer[127] = '\0';

			ImGui::Text("Variable Name:");
			if (ImGui::InputText("##globalVariable", varBuffer, 128)) {
				node.parameters["variable"] = varBuffer;
			}

			float value = node.parameters.count("value") ?
				std::stof(node.parameters["value"]) : 0.f;

			if (ImGui::DragFloat("Value", &value, 0.1f)) {
				node.parameters["value"] = std::to_string(value);
			}

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Global variables are shared across all objects with Visual Logic Component");
		}
		else if (node.imp == NodeImplementation::MODIFY_GLOBAL_FLOAT) {
			ImGui::Text("Modify Global Float Variable");

			std::vector<std::string> globalVarNames = getGlobalVariableNamesFromNodes();

			if (globalVarNames.empty()) {
				ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "No global float variables exist yet.");
				ImGui::Spacing();
				ImGui::TextDisabled("Use 'Set Global Float' node to create one.");
			}
			else {
				std::string currentVar = node.parameters.count("variable") ?
					node.parameters["variable"] : "";

				if (currentVar.empty() ||
					std::find(globalVarNames.begin(), globalVarNames.end(), currentVar) == globalVarNames.end()) {
					currentVar = globalVarNames[0];
					node.parameters["variable"] = currentVar;
				}

				ImGui::Text("Variable:");
				if (ImGui::BeginCombo("##globalVariable", currentVar.c_str())) {
					for (const std::string& varName : globalVarNames) {
						bool isSelected = (currentVar == varName);
						if (ImGui::Selectable(varName.c_str(), isSelected)) {
							node.parameters["variable"] = varName;
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Modify By:");

				float value = node.parameters.count("value") ?
					std::stof(node.parameters["value"]) : 0.f;

				if (ImGui::DragFloat("##value", &value, 0.1f)) {
					node.parameters["value"] = std::to_string(value);
				}

				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Negative to subtract, Positive to add");
			}
		}
		else if (node.imp == NodeImplementation::TRIGGER_BULLET_REFILL_EVENT) {
			ImGui::Text("Trigger Bullet Refill Event on Target");

			std::string targetType = node.parameters.count("targetType") ?
				node.parameters["targetType"] : "all";

			if (ImGui::BeginCombo("Target", targetType.c_str())) {
				const char* options[] = { "all", "collided", "tag", "name", "self" };
				const char* labels[] = { "All Objects", "Collided Object", "By Collision Tag", "By Object Name", "Self Only" };

				for (int i = 0; i < 5; i++) {
					bool isSelected = (targetType == options[i]);

					if (ImGui::Selectable(labels[i], isSelected)) {
						node.parameters["targetType"] = options[i];

						if (!isSelected) {
							node.parameters["targetValue"] = "";
						}
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			// if tag or name, show input for value
			if (targetType == "tag" || targetType == "name") {
				ImGui::Spacing();

				char valueBuffer[128];
				std::string currentValue = node.parameters.count("targetValue") ?
					node.parameters["targetValue"] : "";
				strncpy(valueBuffer, currentValue.c_str(), 127);
				valueBuffer[127] = '\0';

				if (targetType == "tag") {
					ImGui::Text("Collision Tag:");
					if (ImGui::InputText("##targetValue", valueBuffer, 128)) {
						node.parameters["targetValue"] = valueBuffer;
					}

					if (ImGui::BeginCombo("##quickSelectTag", "Quick Select")) {
						std::vector<std::string> uniqueTags = GlobalRegistry::Instance().getCollisionTags();

						for (const std::string& tag : uniqueTags) {
							if (ImGui::Selectable(tag.c_str())) {
								node.parameters["targetValue"] = tag;
							}
						}
						ImGui::EndCombo();
					}
				}
				else if (targetType == "name") {
					ImGui::Text("Object Name:");
					if (ImGui::InputText("##targetValue", valueBuffer, 128)) {
						node.parameters["targetValue"] = valueBuffer;
					}

					if (ImGui::BeginCombo("##quickSelectName", "Quick Select")) {
						auto& cached = CachedGameObjects::Instance().getCachedObjects();

						for (GameObject* cachedObj : cached) {
							if (ImGui::Selectable(cachedObj->getObjectName().c_str())) {
								node.parameters["targetValue"] = cachedObj->getObjectName();
							}
						}
						ImGui::EndCombo();
					}
				}
			}
			else if (targetType == "collided") {
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Note: Only works during collision events");
			}
		}
		else if (node.imp == NodeImplementation::RESTART_LEVEL) {
			ImGui::Text("Restart Level");
			ImGui::Separator();
			ImGui::TextWrapped("Restarts the current level and resets all objects to their spawn positions.");
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "This will:");
			ImGui::BulletText("Reset all object positions");
			ImGui::BulletText("Reset all velocities to zero");
			ImGui::BulletText("Set all states to IDLE");
			ImGui::BulletText("Reset animations to idle");
		}

		else if (node.imp == NodeImplementation::LOAD_LEVEL) {
			ImGui::Text("Load Level");
			ImGui::Separator();

			// Level name input
			char levelBuffer[256];
			std::string currentLevel = node.parameters.count("levelName") ?
				node.parameters["levelName"] : "level01.json";
			strncpy(levelBuffer, currentLevel.c_str(), 255);
			levelBuffer[255] = '\0';

			ImGui::Text("Level File:");
			if (ImGui::InputText("##levelName", levelBuffer, 256)) {
				node.parameters["levelName"] = levelBuffer;
			}

			ImGui::Spacing();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
				"Enter the level filename (e.g., level02.json)");

			// Optional: Quick select dropdown for existing levels
			ImGui::Spacing();
			if (ImGui::BeginCombo("##quickSelectLevel", "Quick Select")) {
				// List common level files
				const char* commonLevels[] = {
					"level01.json",
					"level02.json",
					"level03.json",
					"menu_scene.json"
				};

				for (const char* level : commonLevels) {
					if (ImGui::Selectable(level)) {
						node.parameters["levelName"] = level;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Spacing();
			if (!currentLevel.empty()) {
				ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
					"Will load: %s", currentLevel.c_str());
			}
			else {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f),
					"No level specified!");
			}
			}
	}
	
	/* ---------- END ---------- */


	/* ---------- CONDITION ---------- */
	// check key held
	else if (node.type == NodeType::CONDITION) {
		if (node.imp == NodeImplementation::CHECK_KEY_HELD) {
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
		else if (node.imp == NodeImplementation::CHECK_STATE) {
			//StateMachine* sm = Editor::visualLogicState.targetObj->getComponent<StateMachine>();

			/*if (sm) {*/
			std::string target = node.parameters.count("state") ?
				node.parameters["state"] : JsonIO::stateNames[0];

			if (ImGui::BeginCombo("##checkState", target.c_str())) {
				const std::vector<std::string>& stateNames = JsonIO::stateNames;

				for (const std::string& stateName : stateNames) {
					bool isSelected = (target == stateName);

					if (ImGui::Selectable(stateName.c_str(), isSelected)) {
						node.parameters["state"] = stateName;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::TextDisabled("Remember to add StateMachine to object");
			/*}
			else {
				ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Add StateMachine Component first");
			}*/
		}
		else if (node.imp == NodeImplementation::CHECK_TIMER) {
			ImGui::Text("Check Timer");
			ImGui::Separator();

			std::vector<std::string> availableTimers;
			for (const LogicNode& node2 : Editor::visualLogicState.nodes) {
				if ((node2.imp == NodeImplementation::CHECK_TIMER ||
					node2.imp == NodeImplementation::RESET_TIMER) &&
					node2.parameters.count("timerName")) {

					std::string timer = node2.parameters.at("timerName");

					if (std::find(availableTimers.begin(), availableTimers.end(), timer) == availableTimers.end()) {
						availableTimers.push_back(timer);
					}
				}
			}

			std::string currentTimer = node.parameters.count("timerName") ? node.parameters["timerName"] : "timer_0";

			ImGui::Text("Timer ID:");
			if (ImGui::BeginCombo("##timerName", currentTimer.c_str())) {
				for (const std::string& timer : availableTimers) {
					bool isSelected = (currentTimer == timer);
					if (ImGui::Selectable(timer.c_str(), isSelected)) {
						node.parameters["timerName"] = timer;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			static char thresholdBuffer[64];
			std::string currentThreshold = node.parameters.count("threshold") ?
				node.parameters["threshold"] : "5.0";
			strncpy_s(thresholdBuffer, currentThreshold.c_str(), sizeof(thresholdBuffer) - 1);

			ImGui::Text("Threshold (seconds):");
			if (ImGui::InputText("##threshold", thresholdBuffer, sizeof(thresholdBuffer))) {
				node.parameters["threshold"] = thresholdBuffer;
			}
		}
		else if (node.imp == NodeImplementation::COMPARE_GLOBAL_FLOAT) {
			ImGui::Text("Compare Global Float Variable");

			std::vector<std::string> globalVarNames = getGlobalVariableNamesFromNodes();

			if (globalVarNames.empty()) {
				ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "No global float variables exist yet.");
				ImGui::Spacing();
				ImGui::TextDisabled("Use 'Set Global Float' node to create one.");
			}
			else {
				std::string currentVar = node.parameters.count("variable") ?
					node.parameters["variable"] : "";

				if (currentVar.empty() ||
					std::find(globalVarNames.begin(), globalVarNames.end(), currentVar) == globalVarNames.end()) {
					currentVar = globalVarNames[0];
					node.parameters["variable"] = currentVar;
				}

				ImGui::Text("Variable:");
				if (ImGui::BeginCombo("##globalVariable", currentVar.c_str())) {
					for (const std::string& varName : globalVarNames) {
						bool isSelected = (currentVar == varName);

						if (ImGui::Selectable(varName.c_str(), isSelected)) {
							node.parameters["variable"] = varName;
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Spacing();

				std::string comparison = node.parameters.count("comparison") ?
					node.parameters["comparison"] : "greater";

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

				float threshold = node.parameters.count("threshold") ?
					std::stof(node.parameters["threshold"]) : 0.f;

				if (ImGui::DragFloat("Threshold Value", &threshold, 0.1f)) {
					node.parameters["threshold"] = std::to_string(threshold);
				}

				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Compares with a global variable");
			}
		}
		else if (node.imp == NodeImplementation::CHECK_COLLIDED_STATE) {
			std::string target = node.parameters.count("state") ?
				node.parameters["state"] : JsonIO::stateNames[0];

			if (ImGui::BeginCombo("##checkState", target.c_str())) {
				const std::vector<std::string>& stateNames = JsonIO::stateNames;

				for (const std::string& stateName : stateNames) {
					bool isSelected = (target == stateName);

					if (ImGui::Selectable(stateName.c_str(), isSelected)) {
						node.parameters["state"] = stateName;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::TextDisabled("Remember to add StateMachine to target collided object");
		}
	}
	/* ---------- END ---------- */
}

void VisualLogicEditor::renderSavePopup() {
	if (Editor::visualLogicState.showSavePopup) {
		Editor::visualLogicState.showSavePopup = false;

		if (!Editor::visualLogicState.targetObj) {
			DebugLog::addMessage("No target object to save Visual Logic for.");
			return;
		}

		ImGui::OpenPopup("Save Visual Logic");
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

						// for save & close
						if (Editor::visualLogicState.wantCloseEditor) {
							Editor::visualLogicState.showEditor = false;
							Editor::visualLogicState.isFocus = false;
							clear();
							Editor::visualLogicState.wantCloseEditor = false;
						}

						DebugLog::addMessage("Visual Logic saved as: " + finalFilename + "\n");
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

void VisualLogicEditor::addNode(NodeType type, NodeImplementation imp) {
	LogicNode node;
	
	if (m_showNodeMenu) {
		node.position = m_menuSpawnPos;
	}
	else {
		// get viewport center
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		Vector2D viewportCenter;
		viewportCenter.x = (viewportSize.x / 2.f - m_canvasOffset.x) / m_zoom;
		viewportCenter.y = (viewportSize.y / 2.f - m_canvasOffset.y) / m_zoom;

		node.position = viewportCenter;
	}

	//if (m_selectedNodeId != -1) {
	//	// Find the selected node
	//	auto it = std::find_if(Editor::visualLogicState.nodes.begin(),
	//		Editor::visualLogicState.nodes.end(),
	//		[this](const LogicNode& n) { return n.id == m_selectedNodeId; });

	//	if (it != Editor::visualLogicState.nodes.end()) {
	//		node.position.x = it->position.x + 150.f; // offset to the right
	//		node.position.y = it->position.y + 20.f; // same y position
	//	}
	//	else {
	//		node.position = Vector2D(100, 100);
	//	}
	//}
	//else {
	//	node.position = Vector2D(100, 100);
	//}

	node.id = m_nextNodeId++;
	node.type = type;
	node.imp = imp;
	node.hasInput = (type == NodeType::EVENT) ? false : true; // EVENT no input
	node.hasOutput = (type == NodeType::CONDITION) ? false : true; // CONDITION no output
	node.hasTrueOutput = (type == NodeType::CONDITION) ? true : false; // only for CONDITION
	node.hasFalseOutput = (type == NodeType::CONDITION) ? true : false; // only for CONDITION

	// EVENT nodes
	if (type == NodeType::EVENT) {
		if (imp == NodeImplementation::ON_COLLISION_ENTER || imp == NodeImplementation::WHILE_COLLIDING || imp == NodeImplementation::ON_COLLISION_EXIT) {
			node.parameters["colFilterType"] = "any"; // any/tag/name
			node.parameters["colFilterValue"] = "";
		}
	}
	// ACTION nodes
	else if (type == NodeType::ACTION) {
		if (imp == NodeImplementation::PRINT_MESSAGE)
			node.parameters["message"] = "I love GAM";
		else if (imp == NodeImplementation::MOVE_OBJECT) {
			node.parameters["x"] = "0";
			node.parameters["y"] = "0";
		}
		else if (imp == NodeImplementation::ROTATE_OBJECT) {
			node.parameters["angle"] = "0"; // in degree 
		}
		else if (imp == NodeImplementation::SET_ROTATION) {
			node.parameters["angle"] = "0";
		}
		else if (imp == NodeImplementation::SET_POSITION) {
			node.parameters["x"] = "0";
			node.parameters["y"] = "0";
		}
		else if (imp == NodeImplementation::SET_SCALE) {
			node.parameters["scaleX"] = "1";
			node.parameters["scaleY"] = "1";
		}
		else if (imp == NodeImplementation::SCALE_PROPORTIONAL) {
			node.parameters["scale"] = "2";
		}
		else if (imp == NodeImplementation::SET_FLOAT) {
			node.parameters["variable"] = "shakeDir";
			node.parameters["value"] = "1";
		}
		else if (imp == NodeImplementation::SET_STATE) {
			node.parameters["state"] = JsonIO::stateNames[0];
		}
		else if (imp == NodeImplementation::RESET_TIMER) {
			node.parameters["timerName"] = node.parameters.count("timerName") ? node.parameters["timerName"] : "timer_0";
		}
		else if (imp == NodeImplementation::SET_STATE_ON_COLLIDED) {
			node.parameters["targetStateCollided"] = JsonIO::stateNames[0];
		}
		else if (imp == NodeImplementation::SET_POSITION_ON_COLLIDED) {
			node.parameters["deltaXCollided"] = "1";
			node.parameters["deltaYCollided"] = "1";
		}
		else if (imp == NodeImplementation::APPLY_FORCE_ON_COLLIDED) {
			node.parameters["forceX"] = "0";
			node.parameters["forceY"] = "0";
		}
		else if (imp == NodeImplementation::SET_GLOBAL_FLOAT) {
			node.parameters["variable"] = "var";
			node.parameters["value"] = "0";
		}
		else if (imp == NodeImplementation::MODIFY_GLOBAL_FLOAT) {
			node.parameters["variable"] = "var";
			node.parameters["value"] = "1";
		}
		else if (imp == NodeImplementation::LOAD_LEVEL) {
			node.parameters["levelName"] = "level01.json";
		}
	}
	// CONDITION nodes
	else if (type == NodeType::CONDITION) {
		if (imp == NodeImplementation::CHECK_KEY_HELD) {
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
		else if (imp == NodeImplementation::CHECK_STATE) {
			node.parameters["state"] = JsonIO::stateNames[0];
		}
		else if (imp == NodeImplementation::CHECK_TIMER) {
			node.parameters["timerName"] = node.parameters.count("timerName") ? node.parameters["timerName"] : "timer_0";
			node.parameters["threshold"] = "1.0";
		}
		else if (imp == NodeImplementation::COMPARE_GLOBAL_FLOAT) {
			node.parameters["variable"] = "var";
			node.parameters["comparison"] = "greater";
			node.parameters["threshold"] = "0";
		}
		else if (imp == NodeImplementation::CHECK_COLLIDED_STATE) {
			node.parameters["state"] = JsonIO::stateNames[0];
		}
	}

	Editor::visualLogicState.nodes.push_back(node);

	// select newly created node
	m_selectedNodeId = node.id;
	m_selectedConIndex = -1;
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

	duplicate.hasTrueOutput = it->hasTrueOutput;
	duplicate.hasFalseOutput = it->hasFalseOutput;
	duplicate.hasInput = it->hasInput;
	duplicate.hasOutput = it->hasOutput;

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
			static_cast<int>(Editor::visualLogicState.nodes.size());
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

void VisualLogicEditor::reloadAllVisualLogic(const std::string& scriptName) {
	if (scriptName.empty()) return;

	std::vector<GameObject*> gameObjects = CachedGameObjects::Instance().getCachedObjects();

	int count{};
	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();

			if (logic->filename == scriptName) {
				logic->nodes = Editor::visualLogicState.nodes;
				logic->connections = Editor::visualLogicState.connections;

				// reset runtime state
				logic->hoverNodesChecked = false;
				logic->wasHoveredLastFrame = false;
				logic->currentExePath.clear();
				logic->curCollision.clear();
				logic->preCollision.clear();

				++count;
			}
		}
	}

	if (count > 0) {
		DebugLog::addMessage("Hot-reloaded '" + scriptName + "' for " +
			std::to_string(count) + " object(s)");
	}
	else {
		DebugLog::addMessage("No objects found using '" + scriptName + "'");
	}
}

void VisualLogicEditor::renderNodeSearchMenu() {
	ImGui::Text("Add Node Here");
	ImGui::Separator();

	// search bar (auto-focus on open)
	if (m_showNodeMenu && ImGui::IsWindowAppearing()) {
		ImGui::SetKeyboardFocusHere();
	}

	//bool searchChanged = 
	ImGui::InputTextWithHint("##search", "Search nodes...", m_nodeSearchBuffer, 128);
	ImGui::Separator();

	std::string search = m_nodeSearchBuffer;
	std::transform(search.begin(), search.end(), search.begin(), ::tolower);

	bool hasResults = false;

	if (!search.empty()) {
		// flat list when searching
		for (const auto& node : Editor::allNodes) {
			if (matchesSearch(node.name, search)) {
				if (ImGui::MenuItem(node.name)) {
					addNode(node.type, node.impl);
					ImGui::CloseCurrentPopup();
				}
				hasResults = true;
			}
		}
	}
	else {
		// split by c
		if (ImGui::BeginMenu("Events")) {
			for (const auto& node : Editor::allNodes) {
				if (node.type == NodeType::EVENT) {
					if (ImGui::MenuItem(node.name)) {
						addNode(node.type, node.impl);
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Conditions")) {
			for (const auto& node : Editor::allNodes) {
				if (node.type == NodeType::CONDITION) {
					if (ImGui::MenuItem(node.name)) {
						addNode(node.type, node.impl);
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Actions")) {
			for (const auto& node : Editor::allNodes) {
				if (node.type == NodeType::ACTION) {
					if (ImGui::MenuItem(node.name)) {
						addNode(node.type, node.impl);
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndMenu();
		}
	}

	if (!search.empty() && !hasResults) {
		ImGui::TextDisabled("No nodes found :(");
	}
}


bool VisualLogicEditor::matchesSearch(const std::string& nodeName, const std::string& search) {
	if (search.empty()) return true;

	std::string lowerName = nodeName;
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

	return lowerName.find(search) != std::string::npos;
}

void VisualLogicEditor::renderNodePalette() {
	// EVENTS
	if (ImGui::CollapsingHeader("Events", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (const auto& node : Editor::allNodes) {
			if (node.type == NodeType::EVENT) {
				if (ImGui::Selectable(("  " + std::string(node.name)).c_str())) {
					addNode(node.type, node.impl);
				}
			}
		}
	}

	// CONDITIONS
	if (ImGui::CollapsingHeader("Conditions", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (const auto& node : Editor::allNodes) {
			if (node.type == NodeType::CONDITION) {
				if (ImGui::Selectable(("  " + std::string(node.name)).c_str())) {
					addNode(node.type, node.impl);
				}
			}
		}
	}

	// ACTIONS
	if (ImGui::CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (const auto& node : Editor::allNodes) {
			if (node.type == NodeType::ACTION) {
				if (ImGui::Selectable(("  " + std::string(node.name)).c_str())) {
					addNode(node.type, node.impl);
				}
			}
		}
	}
}

void VisualLogicEditor::verticalSplitter(float* width, bool reverseDir, float minWidth) {
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

	std::string id = "##splitter_" + std::to_string(reinterpret_cast<uintptr_t>(width));
	ImGui::Button(id.c_str(), ImVec2(4.0f, -1));

	ImGui::PopStyleColor(3);

	if (ImGui::IsItemActive()) {
		float delta = ImGui::GetIO().MouseDelta.x;

		if (reverseDir) delta = -delta;
		*width += delta;

		if (*width < minWidth) *width = minWidth;
	}

	if (ImGui::IsItemHovered()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	}

	ImGui::SameLine();
}

std::vector<std::string> VisualLogicEditor::getGlobalVariableNamesFromNodes() {
	std::set<std::string> uniqueVars;  // Use set to avoid duplicates

	for (const LogicNode& node : Editor::visualLogicState.nodes) {
		if (node.imp == NodeImplementation::SET_GLOBAL_FLOAT &&
			node.parameters.count("variable")) {
			std::string varName = node.parameters.at("variable");
			if (!varName.empty()) {
				uniqueVars.insert(varName);
			}
		}
	}

	std::vector<GameObject*> allObjects = CachedGameObjects::Instance().getCachedObjects();

		for (GameObject* obj : allObjects) {
			if (obj->hasComponent<VisualLC>()) {
				VisualLC* logic = obj->getComponent<VisualLC>();

				// Scan this object's nodes for global variables
				for (const LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::SET_GLOBAL_FLOAT &&
						node.parameters.count("variable")) {
						std::string varName = node.parameters.at("variable");
						if (!varName.empty()) {
							uniqueVars.insert(varName);
						}
					}
				}
			}
		}

	return std::vector<std::string>(uniqueVars.begin(), uniqueVars.end());
}

#endif