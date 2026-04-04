/* Start Header ************************************************************************/
/*!
\file       editorManager.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 95%
			Pearly Lin Lee Ying, p.lin, 2401591
			- 5% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      This file implements the core editor UI and viewport functionality for the 
			engine.
		 * It includes classes for:
		 *   - Scene interaction and rendering (SceneWindow, ScenePropertiesWindow)
		 *   - Object management and caching (CachedGameObjects, HierarchyWindow)
		 *   - Property inspection and component editing (InspectorWindow)
		 *   - Asset browsing and management (AssetBrowser)
		 *   - Visual logic editing (VisualLogicEditor)
		 *   - Editor control and menus (MenuBar, AddObjWindow, DebugWindow, PerformanceWindow)
		 *   - Overall editor orchestration (EditorManager)
		 *
		 * Functional purpose:
		 *   Provides a fully integrated environment for creating, editing, and simulating
		 *   game objects and scenes, handling user input, object selection, and UI updates.
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
//#ifdef _DEBUG
#pragma once
#include <imgui.h> //for use of imgui in UISystem
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem
#include <iostream>
#include <algorithm>
#include "Editor/editorState.h"
#include "Editor/gameDebugLog.h"
#include "Editor/undoRedo.h"
#include "Editor/buildSizeAnalyzer.h"
#include "globalRegistry.h"
#include "collision.h"
#include "GameObjectManager.h"
#include "JsonIO.h"
#include "prefabManager.h"
#include "performance.h"
#include "SceneManager.h"
#include "PathManager.h"

#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif

// Window for creating and duplicating GameObjects in the editor
// Functional purpose: Provides a UI for adding new objects (empty, rectangle, circle) or duplicating existing ones
class AddObjWindow {
public:
	// Constructor takes references to shape input states for circle and rectangle
	AddObjWindow(Editor::ShapeControlState& Cshape, Editor::ShapeControlState& Rshape);

	// Render the Add Object window and handle user interactions
	void render(GameObjectManager& manager);

	// Create a new empty GameObject at a specified position (default 0,0,0)
	static void createEmpty(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	// Create a new rectangular GameObject at a specified position
	static void createRec(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	// Create a new circular GameObject at a specified position
	static void createCir(GameObjectManager& manager, float x = 0, float y = 0, float z = 0);

	// Duplicate an existing GameObject, preserving its properties
	static void dupObj(GameObjectManager& manager, GameObject* oriObj);

private:
	// References to editor states for circle and rectangle shape input controls
	Editor::ShapeControlState& m_circleInput;
	Editor::ShapeControlState& m_rectangleInput;
};

// Asset browser that hot-loads assets from disk
// Functional purpose: Allows browsing, loading, and managing assets (create, rename, delete) in the editor
class AssetBrowser {
public:
	// Constructor takes references to asset browser state, object selection state, and menu bar state
	AssetBrowser(Editor::AssetBrowserState& Astate, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate);

	// Render the asset browser UI and handle interactions like selection, drag-drop, popups
	void render();

	// Load or refresh assets from the file system
	void loadAssetsFromDirectory();

	// Convert a file system asset path to a registry-friendly path (e.g., for internal references)
	static std::string toRegistryPath(const std::string& assetPath);

	// Return the currently loaded list of assets (used to check for duplicate names or display)
	const std::vector<Editor::Asset>& getAssets() const { return m_assets; }

private:
	// Popups for managing assets
	void assetDeletePopup();      // Confirm deletion of selected asset
	void assetRenamePopup();      // Rename an asset
	void assetReplacePopup();     // Replace an existing asset with a new one
	void createSubfolderPopup();  // Create a new folder
	void renameSubfolderPopup();  // Rename a folder
	void deleteSubfolderPopup();  // Delete a folder

	// References to various editor states
	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;

	// List of currently loaded assets
	std::vector<Editor::Asset>& m_assets;
};


// Debug console window
// Functional purpose: Displays debugging information and allows debug commands to be executed
class DebugWindow {
public:
	// Constructor specifying which debug mode to use
	DebugWindow(DebugMode Dmode);

	// Render the debug window UI and handle interactions
	void render();

private:
	DebugMode mode; // Current debug mode, determines what info/controls are shown
};

// Window showing the hierarchy of all GameObjects in the scene
// Functional purpose: Allows users to view, select, delete, and manage GameObjects and prefabs
class HierarchyWindow {
public:
	// Constructor takes reference to object selection state for tracking currently selected objects
	HierarchyWindow(Editor::ObjSelectionState& Ostate);

	// Render hierarchy UI and handle user interactions
	void render(GameObjectManager& manager);

private:
	// Handle drag-and-drop operations for prefabs in the hierarchy
	void handlePrefabDragDrop(GameObjectManager& manager);

	// Show right-click context menu for a specific object
	void showContextMenu(GameObjectManager& manager, GameObject* obj, int objIndex);

	// Popup for confirming object deletion
	void objDeletePopup(GameObjectManager& manager);

	// Popup for creating a new prefab from selected objects
	void createPrefabPopup(GameObjectManager& manager);

	// Reference to editor state for currently selected objects
	Editor::ObjSelectionState& m_objSelectionState;
};

// Window for editing properties of selected GameObjects
// Functional purpose: Provides a detailed inspector for modifying components, transforms, and other object properties
class InspectorWindow {
public:
	// Constructor takes references to manager, object selection state, and gizmo state
	InspectorWindow(GameObjectManager& m_manager, Editor::ObjSelectionState& Ostate, Editor::GizmoState& gState);

	// Render inspector UI and handle editing of selected object's properties
	void render();

private:
	// Add new component to the selected GameObject
	void addComponent(GameObject* selected);

	// Render gizmo (position/rotation/scale) control UI
	void renderGizmoCtrl();

	// Render different component property panels
	void renderTransform(GameObject* selected);
	void renderRender(GameObject* selected);
	void renderStateMachine(GameObject* selected);
	void renderCollision(GameObject* selected);
	void renderInput(GameObject* selected);
	void renderPhysics(GameObject* selected, bool canMove);
	void renderFont(GameObject* selected);
	void renderAudio(GameObject* selected);
	void renderTileMap(GameObject* selected);
	void renderParticles(GameObject* selected);
	void renderLua(GameObject* selected);
	void renderVisualLogic(GameObject* selected);
	void renderWaterBody(GameObject* selected);
	void renderWeapon(GameObject* selected);
	void renderButton(GameObject* selected);
	void renderCamera(GameObject* selected);
	void renderVideo(GameObject* selected);
	void renderLayerControls(GameObject* selected);

	// Popup when the user tries to load an unsupported file type
	void wrongFileTypePopup(const std::string& filename);

	// References to editor states
	GameObjectManager& m_manager;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::GizmoState& m_gizmoState;

	// Internal state for editing transforms
	bool m_isEditingPos = false;
	bool m_isEditingRot = false;
	bool m_isEditingScale = false;
	GameObject* m_transformEditObject = nullptr;
};

// Visual Logic Editor for creating node-based logic (like Blueprints)
// Functional purpose: Allows visual programming of object behaviors and game logic
class VisualLogicEditor {
public:
	// Singleton access
	static VisualLogicEditor& Instance() {
		static VisualLogicEditor instance;
		return instance;
	}

	// Delete copy constructor and assignment operator
	VisualLogicEditor(const VisualLogicEditor&) = delete;
	VisualLogicEditor& operator=(const VisualLogicEditor&) = delete;

	// Render the visual logic editor canvas and all nodes/connections
	void render();

	// Clear all logic nodes from editor
	void clear();

	// Load logic graph from a file
	void loadLogic(const std::string& filename);

	// Update the ID for the next node to be added
	void updateNextNodeId();

private:
	VisualLogicEditor();

	int m_nextNodeId;       // Next unique node ID
	int& m_selectedNodeId;  // Currently selected node
	int& m_selectedConIndex; // Currently selected connection index

	// Dragging state for nodes
	bool m_isDragging;
	int m_draggingNodeId;
	ImVec2 m_dragOffset;

	// Canvas state
	ImVec2 m_canvasOffset;
	float m_zoom;

	// Connection creation state
	bool m_isConnecting;
	int m_connectingFromNodeId;
	std::string m_connectingFromPin;
	ImVec2 m_connectionStartPos;

	// Right-click menu & node palette state
	bool m_showNodePalette;
	bool m_showNodeMenu;
	Vector2D m_menuSpawnPos;
	char m_nodeSearchBuffer[128];

	// Save state
	void saveLogic();
	char m_saveFilename[128];

	// Node rendering helpers
	void renderNode(ImDrawList* drawList, LogicNode& node, ImVec2 canvasPos, bool isCanvasHovered);
	void renderConnections(ImDrawList* drawList, ImVec2 canvasPos);
	void renderNodeProperties();
	void renderSavePopup();

	// Node creation
	void addNode(NodeType type, NodeImplementation imp);

	// Utility functions for nodes/connections
	ImU32 getNodeColor(NodeType type);
	bool isPinHovered(ImVec2 pinPos, ImVec2 mousePos);
	void createConnection(int fromNodeID, std::string fromPin, int toNodeID, std::string toPin);

	// Node shortcuts
	void deleteSelectedNode();
	void duplicateSelectedNode();
	void selectNextNode(bool forward);

	// Connection helpers
	ImVec2 getBezierPoint(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float t);
	bool isMouseNearBezier(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3,
		ImVec2 mousePos, float threshold);
	void deleteSelectedConnection();

	// Reload scripts for nodes
	void reloadAllVisualLogic(const std::string& scriptName);

	// Right-click menu and node palette
	void renderNodeSearchMenu();
	bool matchesSearch(const std::string& nodeName, const std::string& search);
	void renderNodePalette();

	// Canvas layout helper
	void verticalSplitter(float* width, bool reverseDir = false, float minWidth = 100.0f);

	// Global variable utilities
	std::vector<std::string> getGlobalVariableNamesFromNodes();
};

// Top menu bar for the editor
// Functional purpose: Provides file, edit, simulation, theme, and tool menus
class MenuBar {
public:
	// Constructor takes references to asset browser, asset/browser/menu/scene states
	MenuBar(AssetBrowser& aBrowser, Editor::AssetBrowserState& Astate, Editor::MenuBarState& Mstate, Editor::ObjSelectionState& Ostate, Editor::SceneState& sceneState);

	// Render the menu bar and handle menu interactions
	void render(GameObjectManager& manager);

	// Simulation controls
	void toggleSimulation(GameObjectManager& manager); // Play or stop
	void pauseResumeSimul(GameObjectManager& manager);

private:
	// Internal menu rendering
	void drawFileMenu(GameObjectManager& manager);
	void drawEditMenu();
	void drawSimulationMenu(GameObjectManager& manager);
	void drawThemeMenu();
	void drawFontMenu();
	void drawToolMenu();

	// Simulation state helpers
	void playSimulation(GameObjectManager& manager);
	void stopSimulation(GameObjectManager& manager);
	void pauseSimulation(GameObjectManager& manager);
	void resumeSimulation(GameObjectManager& manager);

	// Themes
	void themeSinLeDark();
	void themeSinLeLight();
	void themeForest();

	// References to editor subsystems and states
	AssetBrowser& m_assetBrowser;
	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::MenuBarState& m_menuBarState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::SceneState& m_sceneState;
};

// Singleton cache of game objects from GameObjectManager
// Functional purpose: Provides fast access to all GameObjects without repeatedly querying the manager
class CachedGameObjects {
public:
	// Singleton access
	static CachedGameObjects& Instance() {
		static CachedGameObjects instance;
		return instance;
	}

	// Initialize cache with the manager reference
	void initialize(GameObjectManager& manager);

	// Get all cached GameObjects
	const std::vector<GameObject*>& getCachedObjects() const { return m_cachedObjects; }

	// Refresh cache if the underlying manager data changed
	void refresh();

private:
	CachedGameObjects() = default;

	GameObjectManager* m_manager = nullptr;       // Reference to the game object manager
	std::vector<GameObject*> m_cachedObjects;     // Cached pointers to all objects
};

// Window for displaying performance stats (FPS, timings, etc.)
// Functional purpose: Helps the user monitor editor/game performance
class PerformanceWindow {
public:
	void render();
};

// Scene viewport window, shows the main editor camera view
// Functional purpose: Allows users to interact with the 3D scene (select, move, manipulate objects)
class SceneWindow {
public:
	// Constructor initializes window with editor states and references
	SceneWindow(AssetBrowser& aBrowser, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate,
		Editor::SceneWindowState& Sstate, Editor::SceneState& sceneState, Editor::GizmoState& gizmoState);

	// Render the 3D scene with a given texture and aspect ratio
	void render(GLuint texture, float aspectRatio, GameObjectManager& manager);

	// Return current scene name
	std::string getCurrentSceneName() const { return m_sceneState.currentSceneName; }

	// Clear all object selections in this viewport
	void resetSelection();

	// Check if the mouse is currently hovering the scene viewport
	static bool isSceneHovered() { return s_isSceneHovered; }

private:
	/* -------- Rendering helpers -------- */
	void renderGizmo(GameObjectManager& manager);       // Draw gizmo for selected object
	void drawGrid(const glm::mat4& view, const glm::mat4& proj); // Draw scene grid
	void renderObjectHighlight();                       // Highlight hovered or selected objects
	void renderObjCollider(GameObject* obj);           // Draw object colliders for debugging
	void editorCameraControls();                        // Handle camera movement and controls
	/* -------- End Rendering -------- */

	/* -------- Scene management -------- */
	void newScene(GameObjectManager& manager);          // Create a new scene
	void duplicateScene(GameObjectManager& manager);    // Duplicate current scene
	void loadScene(GameObjectManager& manager);         // Load a scene from disk
	/* -------- End Scene management -------- */

	/* -------- Popups for scene actions -------- */
	void newScenePopup(GameObjectManager& manager);
	void duplicateScenePopup(GameObjectManager& manager);
	void loadScenePopup(GameObjectManager& manager);
	/* -------- End Popups -------- */

	/* -------- Object interaction -------- */
	void objPicking();                                  // Handle click selection and collision picking
	void objDragging(GameObjectManager& manager) const; // Handle object dragging in the scene
	/* -------- End Object interaction -------- */

	AssetBrowser& m_assetBrowser;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;
	Editor::SceneWindowState& m_sceneWindowState;
	Editor::SceneState& m_sceneState;
	Editor::GizmoState& m_gizmoState;

	static bool s_isSceneHovered;                       // Track if scene is hovered
};

// Scene-level properties window
// Functional purpose: Shows editable scene-wide properties (lighting, environment, etc.)
class ScenePropertiesWindow {
public:
	ScenePropertiesWindow(GameObjectManager& manager, Editor::SceneState& sceneState)
		: m_manager(manager), m_sceneState(sceneState) {
	}

	// Render scene properties UI
	void render();

private:
	GameObjectManager& m_manager;
	Editor::SceneState& m_sceneState;
};




// the big laoban for ALL editor viewports
// Main editor manager coordinating all editor viewports and UI windows
// Functional purpose: Handles rendering, updating, input, and communication between editor windows
class EditorManager {
public:
	EditorManager(GameObjectManager& manager);

	/* -------- Update & render editor viewports -------- */
	void update();                                      // Update editor states, handle input
	void renderScene(GLuint texture, float aspectRatio); // Render the main scene viewport
	/* -------- End Update & render -------- */

	// Flag that assets have changed and UI should reload them
	static void assetChanged() { assetsChangedFlag = true; }

	// Editing mode toggles (true = user editing scene, no simulation running)
	static bool isEditingMode() { return isEditing; }
	static void toggleEditing(bool e) { isEditing = e; }
	static void setCanToggleEditor(bool v) { cantoggleEditor = v; }

	// Return position and size of the scene viewport
	static void getScenePosNSize(ImVec2& scenePos, ImVec2& sceneSize) {
		scenePos = Editor::sceneWindowState.scenePos;
		sceneSize = Editor::sceneWindowState.sceneSize;
	}

	// Simulation pause state
	static bool isPaused() { return pausedSimul; }
	static void togglePause(bool e) { pausedSimul = e; }

	// Whether the editor can currently toggle editing mode
	static bool canToggleEditor() { return cantoggleEditor; }

	// Access build size analyzer
	static BuildSizeAnalyzer& getBuildAnalyzer() { return s_buildAnalyzer; }

private:
	// Handle editor keyboard shortcuts
	void handleShortcutKeys(GameObjectManager& manager);

	// Editor UI windows/components
	AddObjWindow addObjWindow;
	AssetBrowser assetBrowser;
	DebugWindow editorDebugWindow;
	DebugWindow playDebugWindow;
	HierarchyWindow hierarchyWindow;
	InspectorWindow inspectorWindow;
	MenuBar menuBar;
	PerformanceWindow performanceWindow;
	SceneWindow sceneWindow;
	ScenePropertiesWindow scenePropertiesWindow;
	static inline BuildSizeAnalyzer s_buildAnalyzer;

	GameObjectManager& m_manager;

	// Static editor state flags
	static inline bool isEditing = false;
	static inline bool pausedSimul = false;
	static inline bool assetsChangedFlag = true;
	static inline bool cantoggleEditor = false;
};