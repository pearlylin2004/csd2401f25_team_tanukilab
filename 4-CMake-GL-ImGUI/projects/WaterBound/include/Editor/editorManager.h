/* Start Header ************************************************************************/
/*!
\file       ui.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 90%
			Pearly Lin Lee Ying, p.lin, 2401591
			- 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 24th, 2025
\brief      Declaration of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <imgui.h> //for use of imgui in UISystem
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem
#include <iostream>
#include <algorithm>
#include "Editor/editorState.h"
#include "Editor/gameDebugLog.h"
#include "Editor/themeManager.h"
#include "collision.h"
#include "GameObjectManager.h"
#include "JsonIO.h"
#include "prefabManager.h"
#include "performance.h"

#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/assets"
#endif

class AddObjWindow {
public:
	AddObjWindow(Editor::ShapeControlState& Cshape, Editor::ShapeControlState& Rshape);
	void render(GameObjectManager& manager);

private:
	Editor::ShapeControlState& m_circleInput;
	Editor::ShapeControlState& m_rectangleInput;
};

class AssetBrowser {
public:
	AssetBrowser(Editor::AssetBrowserState& Astate, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate);

	void render();
	void loadAssetsFromDirectory();

	// return the loaded list of assets (to check duplicated name when creating new scene)
	const std::vector<Editor::Asset>& getAssets() const { return m_assets; }

private:
	void assetDeletePopup();
	void assetRenamePopup();
	void assetReplacePopup();

	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;
	std::vector<Editor::Asset> m_assets;
};

class DebugWindow {
public:
	DebugWindow(DebugMode Dmode);
	void render();

private:
	DebugMode mode;
};

class HierarchyWindow {
public:
	HierarchyWindow(Editor::ObjSelectionState& Ostate);
	void render(GameObjectManager& manager);

private:
	void handlePrefabDragDrop(GameObjectManager& manager);
	void showContextMenu(GameObject* obj, int objIndex);

	void objDeletePopup(GameObjectManager& manager);
	void createPrefabPopup(GameObjectManager& manager);

	Editor::ObjSelectionState& m_objSelectionState;
};

class InspectorWindow {
public:
	InspectorWindow(Editor::ObjSelectionState& Ostate);
	void render(GameObjectManager& manager);

private:
	void renderTransform(Transform* transform);
	void renderRender(Render* render, GameObject* selected);

	Editor::ObjSelectionState& m_objSelectionState;
};

class MenuBar {
public:
	MenuBar(AssetBrowser& aBrowser, Editor::AssetBrowserState& Astate, Editor::MenuBarState& Mstate, Editor::ObjSelectionState& Ostate, Editor::SceneState& sceneState);
	void render(GameObjectManager& manager);
	void toggleSimulation(GameObjectManager& manager);

private:
	void drawFileMenu(GameObjectManager& manager);
	void drawSimulationMenu(GameObjectManager& manager);
	void drawThemeMenu();

	void playSimulation(GameObjectManager& manager);
	void stopSimulation(GameObjectManager& manager);
	void themeSinLeDark();
	void themeSinLeLight();
	void themeForest();

	AssetBrowser& m_assetBrowser;
	Editor::AssetBrowserState& m_AssetBrowserState;
	Editor::MenuBarState& m_menuBarState;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::SceneState& m_sceneState;
};

class PerformanceWindow {
public:
	void render();
};

class SceneWindow {
public:
	SceneWindow(AssetBrowser& aBrowser, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate, Editor::SceneWindowState& Sstate, Editor::SceneState& sceneState);

	void render(GLuint texture, float aspectRatio, GameObjectManager& manager);

	/* -------- Object Click and Drag -------- */
	// handle click and select logic and collision
	void objPicking(GameObjectManager& manager);

	// handle dragging of the obj (obj will follow cursor)
	void objDragging() const;
	/* -------- END -------- */

	std::string getCurrentSceneName() const { return m_sceneState.currentSceneName; }

private:
	void newScene(GameObjectManager& manager);

	// dup current scene and open
	void duplicateScene(GameObjectManager& manager);

	// load existing scene
	void loadScene(GameObjectManager& manager);

	/* --------- Pop up Modals -------- */

	void newScenePopup(GameObjectManager& manager);
	void duplicateScenePopup(GameObjectManager& manager);
	void loadScenePopup(GameObjectManager& manager);

	/* -------- END -------- */

	AssetBrowser& m_assetBrowser;
	Editor::ObjSelectionState& m_objSelectionState;
	Editor::MenuBarState& m_menuBarState;
	Editor::SceneWindowState& m_sceneWindowState;
	Editor::SceneState& m_sceneState;
};




// the big laoban for ALL editor viewports
class EditorManager {
public:
	EditorManager();

	/* -------- Render all editor viewports ------- */
	void update(GameObjectManager& manager);
	void renderScene(GLuint texture, float aspectRatio, GameObjectManager& manager);
	/* -------- END -------- */

	static void assetChanged() { assetsChangedFlag = true; } // set to true to reload assets in UI
	static bool isEditingMode() { return isEditing; }
	static void toggleEditing(bool e) { isEditing = e; } // if true then editing mode, dun play animation, obj dun move

private:
	void handleShortcutKeys(GameObjectManager& manager);

	AssetBrowser assetBrowser;
	DebugWindow editorDebugWindow;
	DebugWindow playDebugWindow;
	HierarchyWindow hierarchyWindow;
	InspectorWindow inspectorWindow;
	MenuBar menuBar;
	PerformanceWindow performanceWindow;
	SceneWindow sceneWindow;

	static inline bool isEditing = false;
	static inline bool assetsChangedFlag = true;
};
