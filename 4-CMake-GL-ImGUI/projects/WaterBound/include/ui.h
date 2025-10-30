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
#include <imgui.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include "collision.h" //for using circles
#include "GameObjectManager.h"
#include "input.h"
#include "performance.h" //for performance tracking

// imma replicate a very scuffed version of Unity :) call me tian cai
class UI {
public:
	UI(); // only used to initialize shape presets, can be =DEFAULT later when loading from json
	~UI() = default;

	void update(GameObjectManager& manager);

	// debug message logging
	static void addDebugMessage(const std::string& msg);

	// edit shape windows
	void shapeControl(Transform& t1, Transform& t2);

	void loadAssetsFromDirectory();

private:
	void drawPerformance();
	void drawShapes(GameObjectManager& manager);
	//void drawControls(std::vector<Collision::Circle>& circles);
	void drawDebug();
	void objListPanel(GameObjectManager& manager);
	void objPropertyPanel(GameObjectManager& manager);
	void objInspectWindow(GameObjectManager& manager);
	void assetWindow();


	struct PanelState {
		float objListWidth = 250.0f; // Width of the object list panel
		float objListHeight = -1.0f; // Height of the object list panel, -1 means full height
		float objListMaxWidth = 400.0f;
		float objListMinWidth = 150.0f;

		float splitterSize = 5.0f;  // Size of the splitter between panels
		int maxObjects = 10; // max objects to show in list without scrolling
	} m_PanelState;

	struct SelectionState {
		GameObject* selectedObject = nullptr; // currently selected object
		int selectedIndex = -1; // index of selected object in the list

		bool aspectRatioLock = true; // lock aspect ratio for resizing
		bool ratioSet = false;
		float ratioY = 1.0f;
		float ratioZ = 1.0f;
	} m_SelectionState;

	struct shapeControlState {
		float inputX = 0.0f;
		float inputY = 0.0f;
		float inputZ = 0.0f;
	} circleInput, RectangleInput;

	enum class ShapeType {
		Leaves,
		Bullet
	};

	struct Range {
		float min;
		float max;
	};

	// might be override by the actual preset in the future
	struct ShapeDefaults {
		float posX = 0.f;
		float posY = 0.f;
		float posZ = 0.f;
		float rotation = 0.f;
		float scaleX = 0.f;
		float scaleY = 0.f;
		float scaleZ = 0.f;
	};

	struct Asset {
		std::string name;
		std::string path;
		std::string folder; // in case there is a parent folder (e.g. textures/) for filtering in the future
	};

	struct AssetBrowserState {
		std::string currentFolder = "shaders/"; // we dont have a asset folder yet :(

		float iconSize = 80.0f;
		//float buttonPadding = 20.0f; // adding padding made my code ugly
	} m_AssetBrowserState;;

	static inline std::vector<std::string> m_debugMessages;

	std::unordered_map<ShapeType, ShapeDefaults> m_shapePresets;
	std::vector<Asset> m_assets;

};

#endif