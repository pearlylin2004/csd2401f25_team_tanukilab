#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <imgui.h>
#include "GameObjectManager.h"

namespace Editor {
	/* ----------------- Menu Bar -----------------*/
	struct MenuBarState {
		bool showNewScenePopup = false;
		bool showLoadScenePopup = false;
		bool showDuplicatePopup = false;

		std::string sceneToLoad;
	};
	/* ----------------- Menu Bar End -----------------*/


	/* ----------------- Game Object Selection -----------------*/
	struct ObjSelectionState {
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
	};
	/* ----------------- Game Object Selection End -----------------*/


	struct ShapeControlState {
		float inputX = 0.0f;
		float inputY = 0.0f;
		float inputZ = 0.0f;
	};


	/* ----------------- Scene Viewport -----------------*/
	struct SceneWindowState {
		ImVec2 scenePos;  // top left corner of viewport
		ImVec2 sceneSize; // size of viewport
	};
	/* ----------------- Scene Viewport End -----------------*/

	/* ----------------- Assets -----------------*/
	// migrate to asset manager in the future
	struct Asset {
		std::string name;
		std::string path;
		std::string folder; // in case there is a parent folder (e.g. textures/) for filtering in the future
	};

	struct AssetBrowserState {
		std::string currentFolder = "assets/"; // we dont have a asset folder yet :(
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
	};
	/* ----------------- Assets End -----------------*/

	
	/* ----------------- Scene State ---------------- */
	struct SceneState {
		std::string currentSceneName = "level01.json";
		std::string tempSceneName = "temp_scene.json"; // for simulation
	};

	inline MenuBarState menuBarState;
	inline ObjSelectionState objSelectionState;
	inline ShapeControlState circleInput;
	inline ShapeControlState rectangleInput;
	inline SceneWindowState sceneWindowState;
	inline AssetBrowserState assetBrowserState;
	inline SceneState sceneState;
}