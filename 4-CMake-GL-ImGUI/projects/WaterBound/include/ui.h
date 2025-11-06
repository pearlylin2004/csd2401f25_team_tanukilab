/* Start Header ************************************************************************/
/*!
\file       ui.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
			- 100% of the file
\par        p.lin@digipen.edu
\date       September, 24th, 2025
\brief      Declaration of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef UI_H
#define UI_H
#include <imgui.h> //for use of imgui in UISystem
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include "GameObjectManager.h"
#include "collision.h"
#include "input.h"
#include "performance.h"
#include "prefabManager.h"

class UI {
public:
	UI(); // only used to initialize shape presets, can be =DEFAULT later when loading from json
	~UI() = default;

	void update(GameObjectManager& manager);

	// debug message logging
	static void addDebugMessage(const std::string& msg);
	static void assetChanged() { assetsChangedFlag = true; } // set to true to reload assets in UI
	static bool isEditingMode() { return isEditing; }
	static void toggleEditing(bool e) { isEditing = e; } // if true then editing mode, dun play animation, obj dun move

	// load assets from asset directory (when flagged)
	void loadAssetsFromDirectory();

	void sceneWindow(GLuint texture, float aspectRatio);

private:
	void drawPerformance();
	void drawShapes(GameObjectManager& manager);
	void drawDebug();

	void editorMenuBar(GameObjectManager& manager);
	void objHierarchyWindow(GameObjectManager& manager);
	void objInspectorWindow(GameObjectManager& manager);
	void assetWindow();
	void editorCameraControls();

	void newScenePopup(GameObjectManager& manager);
	void duplicateScenePopup(GameObjectManager& manager);
	void loadScenePopup(GameObjectManager& manager);

	void objDeletePopup(GameObjectManager& manager);
	void createPrefabPopup(GameObjectManager& manager);

	void newScene(GameObjectManager& manager);
	void duplicateScene(GameObjectManager& manager);
	void loadScene(GameObjectManager& manager);

	void assetDeletePopup();
	void assetRenamePopup();
	void assetReplacePopup();

	void objPicking(GameObjectManager& manager);
	void objDragging() const;

	void themeSinLeDark();
	void themeSinLeLight();
	void themeForest();

	/* ----------------- Menu Bar -----------------*/
	struct menuBarState {
		bool showNewScenePopup = false;
		bool showLoadScenePopup = false;
		bool showDuplicatePopup = false;

		std::string sceneToLoad;
	} m_MenuBarState;
	/* ----------------- Menu Bar End -----------------*/


	/* ----------------- Game Object Selection -----------------*/
	struct SelectionState {
		GameObject* selectedObject = nullptr; // currently selected object
		int selectedIndex = -1; // index of selected object in the list

		std::unique_ptr<GameObject> selectedPrefab;

		GameObject* draggedObject = nullptr; // currently dragged object

		bool aspectRatioLock = true; // lock aspect ratio for resizing
		bool ratioSet = false;
		float ratioY = 1.0f;
		float ratioZ = 1.0f;

		bool showDeletePopup = false;
		bool showCreatePrefabPopup = false;
	} m_SelectionState;
	/* ----------------- Game Object Selection End -----------------*/


	/* ----------------- Creating Game Object in Shapes -----------------*/
	struct shapeControlState {
		float inputX = 0.0f;
		float inputY = 0.0f;
		float inputZ = 0.0f;
	} circleInput, RectangleInput;
	/* ----------------- Creating Game Object in Shapes End -----------------*/


	/* ----------------- Scene Viewport -----------------*/
	struct SceneWindowState {
		ImVec2 scenePos;  // top left corner of viewport
		ImVec2 sceneSize; // size of viewport
	} m_SceneWindowState;
	/* ----------------- Scene Viewport End -----------------*/

	/* ----------------- Assets -----------------*/
	// migrate to asset manager in the futre
	struct Asset {
		std::string name;
		std::string path;
		std::string folder; // in case there is a parent folder (e.g. textures/) for filtering in the future
	};

	struct AssetBrowserState {
		std::string currentFolder = "Debug/shaders/"; // we dont have a asset folder yet :(
		std::filesystem::file_time_type lastTopLevelRefresh; // top level folder timestamp
		std::filesystem::file_time_type lastRefresh;
		
		// for deleting asset via browser
		std::string fullPathToDelete;
		bool showDeletePopup = false;

		// for renaming asset via browser
		std::string fullPathToRename;
		bool showRenamePopup = false;
		char newNameBuffer[128] = "\0";

		std::string fullPathToReplace;
		bool showReplacePopup = false;
		char replacePathBuffer[128] = "\0";

		float iconSize = 100.0f;
		float buttonPadding = 20.0f; // adding padding made my code ugly
	} m_AssetBrowserState;
	/* ----------------- Assets End -----------------*/


	static inline std::vector<std::string> m_debugMessages;
	static bool isEditing; // true means editing and game obj not movements

	std::vector<Asset> m_assets;
	static bool assetsChangedFlag; // whether need to update asset browser

	std::string m_currentSceneName; // filename of the current scene
	std::string m_tempSceneName; // a temp file to store the data while playing so it wont affect the actual scene data
	bool m_isPlaying; // to track Play/Stop mode
};

#endif