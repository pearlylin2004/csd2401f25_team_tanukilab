/* Start Header ************************************************************************/
/*!
\file       input.h
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
#include "collision.h" //for using circles
#include "GameObjectManager.h"
#include "input.h"

class UI {
public:
	UI(); // only used to initialize shape presets, can be =DEFAULT later when loading from json
	~UI() = default;

	void update(GameObjectManager& manager);

	static void addDebugMessage(const std::string& msg);

	// edit shape windows
	void shapeControl(Transform& t1, Transform& t2);

private:
	void drawPerformance();
	void drawShapes(GameObjectManager& manager);
	//void drawControls(std::vector<Collision::Circle>& circles);
	void drawDebug();
	void objListPanel(GameObjectManager& manager);
	void objPropertyPanel(GameObjectManager& manager);
	void objInspectWindow(GameObjectManager& manager);


	struct ObjUiState {
		float objListWidth = 250.0f; // Width of the object list panel
		float objListHeight = -1.0f; // Height of the object list panel, -1 means full height
		float objListMaxWidth = 600.0f;
		float objListMinWidth = 150.0f;
		float splitterSize = 5.0f;  // Size of the splitter between panels
		int maxObjects = 10; // max objects to show in list without scrolling

		GameObject* selectedObject = nullptr; // currently selected object
		int selectedIndex = -1; // index of selected object in the list
	};

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
		float rotation = 0.f;
		float scaleX = 0.f;
		float scaleY = 0.f;
		float scaleZ = 0.f;

		// once json done, auto use this if not overriden by json
		// consider having like some reusable RANGE for certain game obj?
		// OPTIONAL slider ranges (min, max)
		Range posXRange = { -15.f, 15.f };
		Range posYRange = { -10.f, 10.f };
		Range rotRange = { 0.f, 360.f };
		Range scaleXRange = { 0.1f, 1.f };
		Range scaleYRange = { 0.1f, 1.f };
	};

	static inline std::vector<std::string> m_debugMessages;


	float xAdd = 0.0f, yAdd = 0.0f; //temp
	std::unordered_map<ShapeType, ShapeDefaults> m_shapePresets;
	ObjUiState m_ObjUIState;
};

//namespace UI{
//	extern std::vector<std::string> debugMessages;
//    extern float xAdd, yAdd; //temp
//
//	struct ObjUiState {
//		float objListWidth = 250.0f; // Width of the object list panel
//		float splitterSize = 5.0f;  // Size of the splitter between panels
//		GameObject* selectedObject = nullptr; // currently selected object
//		int selectedIndex = -1; // index of selected object in the list
//	};
//
//    enum class ShapeType { 
//        Leaves, 
//        Bullet 
//    };
//
//	struct Range {
//		float min;
//		float max;
//	};
//
//    struct ShapeDefaults {
//		float posX = 0.f;
//		float posY = 0.f;
//		float rotation = 0.f;
//		float scaleX = 0.f;
//		float scaleY = 0.f;
//		float scaleZ = 0.f;
//
//		// once json done, auto use this if not overriden by json
//		// consider having like some reusable RANGE for certain game obj?
//		// OPTIONAL slider ranges (min, max)
//		Range posXRange = { -15.f, 15.f };
//		Range posYRange = { -10.f, 10.f };
//		Range rotRange = { 0.f, 360.f };
//		Range scaleXRange = { 0.1f, 1.f };
//		Range scaleYRange = { 0.1f, 1.f };
//	};
//
//	extern std::unordered_map<ShapeType, ShapeDefaults> shapePresets;
//
//	// draw UI windows
//    void drawPerformance();
//	void drawShapes(GameObjectManager& manager);
//    //void drawControls(std::vector<Collision::Circle>& circles);
//	void drawDebug();
//	void objListWindow(GameObjectManager& manager);
//	void objPropertyWindow(GameObjectManager& manager);
//	void objInspectWindow(GameObjectManager& manager);
//
//	// edit shape windows
//    void shapeControl(Transform& t1, Transform& t2);
//    //void dockingSetUp();
//}
#endif