/* Start Header ************************************************************************/
/*!
\file       editorState.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Central namespace defining the editor's runtime state, UI state, and component registry.
 *
 * This file contains structures and variables that track the state of the editor, including:
 * - Menu bar popups and commands (`MenuBarState`)
 * - Current object selection and prefab handling (`ObjSelectionState`)
 * - Shape input and control states (`ShapeControlState`)
 * - Scene viewport information (`SceneWindowState`)
 * - Asset management and asset browser state (`Asset`, `AssetBrowserState`)
 * - Current scene management (`SceneState`)
 * - Lua script state for scripting support (`ScriptState`)
 * - Popups for file validation or messages (`PopupState`)
 * - Gizmo operation state for object transformation (`GizmoState`)
 * - Visual logic editor nodes, connections, and editor UI (`VisualLogicState`, `VLC` constants)
 * - Node definitions for the visual logic system (`NodeDef`, `allNodes`)
 * - Component registry for runtime component management (`ComponentEntry`, `componentRegistry`)
 *
 * The namespace also includes helper variables and functions:
 * - `hasCam` indicates whether a camera component currently exists in the scene
 * - `findEntry()` retrieves a `ComponentEntry` from the registry by name
 *
 * @note Many structs include temporary UI buffers for renaming, creating, or deleting objects/folders.
 * @note Designed for use in an ImGui-based editor environment with runtime component addition/removal.
 * @note Components registered here include Render, Collision, Physics, Input, State Machine, Font,
 *       Audio, Tile Map, Lua Script, Particle, Visual Logic, Animation, Water Body, Weapon, Button,
 *       Camera, and Video.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
//#ifdef _DEBUG
#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include "GameObjectManager.h"
#include "prefabManager.h"
#include "JsonIO.h"

namespace Editor {
	inline bool hasCam = false;

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

		//bool aspectRatioLock = false; // lock aspect ratio for resizing
		//bool ratioSet = false;
		//float ratioY = 1.0f;
		//float ratioZ = 1.0f;

		bool showDeletePopup = false;
		bool showCreatePrefabPopup = false;
	};
	/* ----------------- Game Object Selection End -----------------*/


	/* ----------------- Shape Control -----------------*/
	struct ShapeControlState {
		float inputX = 0.0f;
		float inputY = 0.0f;
		float inputZ = 0.0f;
	};
	/* ----------------- Shape Control End -----------------*/


	/* ----------------- Scene Viewport -----------------*/
	struct SceneWindowState {
		ImVec2 scenePos;  // top left corner of viewport
		ImVec2 sceneSize; // size of viewport

		bool showSingleCollider = false;
		bool showAllColliders = false;
	};
	/* ----------------- Scene Viewport End -----------------*/

	/* ----------------- Assets -----------------*/
	// migrate to asset manager in the future
	struct Asset {
		std::string name;
		std::string path;
		std::string folder; // in case there is a parent folder (e.g. textures/) for filtering
	};

	struct AssetBrowserState {
		std::string currentFolder = "assets/"; // we have an asset folder yayy
		std::filesystem::file_time_type lastTopLevelRefresh; // top level folder timestamp
		std::filesystem::file_time_type lastRefresh;
		std::string currentViewFolder;
		std::vector<std::string> subFolders;
		std::vector<PrefabUsageInfo> prefabUsageInfo; // for tracking prefab usage info in delete popup
		std::vector<Editor::Asset> assets;

		// creating subfolder
		bool showCreateSubfolderPopup = false;
		bool showRenameSubfolderPopup = false;
		bool showDeleteSubfolderPopup = false;
		char newSubfolderNameBuffer[128] = {};
		char renameFolderBuffer[128] = {};
		std::vector<std::string> currentViewSubFolders; // subfolders inside currentViewFolder
		std::string folderToRename;
		std::string folderToDelete;

		// for dropping of files from file explorer
		std::string hoverFolder;

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
		std::string currentSceneName = "";
		std::string tempSceneName = "temp_scene.json"; // for simulation
	};
	/* ----------------- Scene End -----------------*/


	/* ----------------- Lua Script State -----------------*/
	struct ScriptState {
		std::vector<std::string> availScripts;
		std::string scriptDir = "assets/scripting/";
	};
	/* ----------------- Script End -----------------*/


	/* ----------------- Popup End -----------------*/
	struct PopupState {
		bool audioFileTypePopup = false;
		bool textureFileTypePopup = false;
		bool videoFileTypePopup = false;
		bool prefabFileTypePopup = false;
		std::string filename;
		std::string message;
	};
	/* ----------------- Popup End -----------------*/


	/* ----------------- Gizmo ---------------- */
	enum GizmoOperation {
		GIZMO_NONE = -1,
		GIZMO_TRANSLATE = ImGuizmo::TRANSLATE,
		GIZMO_ROTATE = ImGuizmo::ROTATE,
		GIZMO_SCALE = ImGuizmo::SCALE
	};

	struct GizmoState {
		GizmoOperation currentOp = GIZMO_NONE; // ImGuizmo::OPERATION
		ImGuizmo::MODE currentMode = ImGuizmo::WORLD; // do not change

		// snapping
		bool useSnap = false;
		float snapValues[3] = { 1.f, 1.f, 1.f };
		float snapAngle = 15.f;

		// grid
		bool showGrid = false;
		//int gridSpacing = 1;
		Vector2D gridOffset{ 0.0f, 0.0f };
	};
	/* ----------------- Gizmo End -----------------*/


	/* ----------------- Visual Logic Editor ---------------- */
	struct VisualLogicState {
		int selectedNodeId = -1;
		int selectedConIndex = -1;

		bool showEditor = false;
		bool isFocus = false; // whether the window is focused
		bool wantCloseEditor = false;
		std::string currentGraphName = "";
		GameObject* targetObj = nullptr;

		bool showSavePopup = false;

		std::vector<LogicNode> nodes;
		std::vector<LogicConnection> connections;

		float nodePaletteWidth = 200.f;
		float propertiesWidth = 250.f;
	};

	namespace VLC {
		// nodes
		const float NODE_WIDTH = 250.f;
		const float NODE_HEIGHT = 80.f;
		const float NODE_PADDING = 10.f;

		// pins
		const float PIN_RADIUS = 6.f;
		const float PIN_Y_OFFSET = 30.f;
		const float PIN_TEXT_OFFSET = 12.f;
		const float PIN_TEXT_Y_OFFSET = 8.f;
		const float PIN_TRUE_Y = 30.f;
		const float PIN_FALSE_Y = 50.f;
		const float PIN_OFFSET_Y = 10.f;  // for true/false

		// connections
		const float CONNECTION_Y_CENTER = 40.f;
		const float CONNECTION_BEZIER_OFFSET = 50.f;

		// border & lines
		const float BORDER_RADIUS = 4.f;
		const float BORDER_THIN = 1.f;
		const float BORDER_THICK = 3.f;
		const float LINE_NORMAL = 2.f;
		const float LINE_HOVERED = 3.5f;
		const float LINE_SELECTED = 4.f;

		// delete button
		const float DELETE_SIZE = 20.f;
		const float DELETE_OFFSET_X = 25.f;
		const float DELETE_OFFSET_Y = 5.f;
		const float DELETE_TEXT_OFFSET_X = 6.f;
		const float DELETE_TEXT_OFFSET_Y = 2.f;

		// grid
		const float GRID_SIZE = 64.f;
	}

	struct NodeDef {
		const char* name;
		NodeType type;
		NodeImplementation impl;
	};

	static const NodeDef allNodes[] = {
		// EVENTS
		{"On Start", NodeType::EVENT, NodeImplementation::ON_START},
		{"On Update", NodeType::EVENT, NodeImplementation::ON_UPDATE},
		{"On Collision Enter", NodeType::EVENT, NodeImplementation::ON_COLLISION_ENTER},
		{"While Collision", NodeType::EVENT, NodeImplementation::WHILE_COLLIDING},
		{"On Collision Exit", NodeType::EVENT, NodeImplementation::ON_COLLISION_EXIT},
		{"On Hover Enter", NodeType::EVENT, NodeImplementation::ON_HOVER_ENTER},
		{"While Hovering", NodeType::EVENT, NodeImplementation::WHILE_HOVERING},
		{"On Hover Exit", NodeType::EVENT, NodeImplementation::ON_HOVER_EXIT},
		{"On Click Enter", NodeType::EVENT, NodeImplementation::ON_CLICK_ENTER},
		{"While Clicking", NodeType::EVENT, NodeImplementation::WHILE_CLICKING},
		{"On Click Exit", NodeType::EVENT, NodeImplementation::ON_CLICK_EXIT},
		{"On Bullet Shoot", NodeType::EVENT, NodeImplementation::ON_BULLET_SHOOT},
		{"On Bullet Refill", NodeType::EVENT, NodeImplementation::ON_BULLET_REFILL},

		// CONDITIONS
		{"Is Key Held", NodeType::CONDITION, NodeImplementation::CHECK_KEY_HELD},
		{"Compare Value", NodeType::CONDITION, NodeImplementation::COMPARE_VALUE},
		{"Compare Variable", NodeType::CONDITION, NodeImplementation::COMPARE_VAR},
		{"Check State", NodeType::CONDITION, NodeImplementation::CHECK_STATE},
		{"Check Timer", NodeType::CONDITION, NodeImplementation::CHECK_TIMER},
		{"Compare Global Float", NodeType::CONDITION, NodeImplementation::COMPARE_GLOBAL_FLOAT},
		{"Check Collided State", NodeType::CONDITION, NodeImplementation::CHECK_COLLIDED_STATE},
		{ "Check Animation End", NodeType::CONDITION, NodeImplementation::CHECK_ANIMATION_END },
		{ "Is Video End", NodeType::CONDITION, NodeImplementation::IS_VIDEO_END },
		
		// ACTIONS
		{"Print Message", NodeType::ACTION, NodeImplementation::PRINT_MESSAGE},
		{"Move Object", NodeType::ACTION, NodeImplementation::MOVE_OBJECT},
		{"Set Position", NodeType::ACTION, NodeImplementation::SET_POSITION},
		{"Rotate Object", NodeType::ACTION, NodeImplementation::ROTATE_OBJECT},
		{"Set Rotation Rate of Change", NodeType::ACTION, NodeImplementation::SET_ROTATION_RATE_OF_CHANGE},
		{"Set Rotation", NodeType::ACTION, NodeImplementation::SET_ROTATION},
		//{"Rotate To", NodeType::ACTION, NodeImplementation::ROTATE_TO},
		{"Set Scale", NodeType::ACTION, NodeImplementation::SET_SCALE},
		{"Scale Proportional", NodeType::ACTION, NodeImplementation::SCALE_PROPORTIONAL},
		{"Set Float", NodeType::ACTION, NodeImplementation::SET_FLOAT},
		{"Modify Float Variable", NodeType::ACTION, NodeImplementation::MODIFY_FLOAT},
		{"Play Sound", NodeType::ACTION, NodeImplementation::PLAY_SOUND},
		{"Set State", NodeType::ACTION, NodeImplementation::SET_STATE},
		{"Reset Timer", NodeType::ACTION, NodeImplementation::RESET_TIMER},
		{"Set Sprite", NodeType::ACTION, NodeImplementation::SET_TEXTURE},
		{"Set Position on Collider", NodeType::ACTION, NodeImplementation::SET_POSITION_ON_COLLIDED},
		{"Set State on Collider", NodeType::ACTION, NodeImplementation::SET_STATE_ON_COLLIDED},
		{"Set Force on Collider", NodeType::ACTION, NodeImplementation::APPLY_FORCE_ON_COLLIDED},
		{"Set Global Float", NodeType::ACTION, NodeImplementation::SET_GLOBAL_FLOAT},
		{"Modify Global Float", NodeType::ACTION, NodeImplementation::MODIFY_GLOBAL_FLOAT},
		{"Trigger Bullet Refill Event", NodeType::ACTION, NodeImplementation::TRIGGER_BULLET_REFILL_EVENT},
		{"Restart Level", NodeType::ACTION, NodeImplementation::RESTART_LEVEL},
		{"Load Level", NodeType::ACTION, NodeImplementation::LOAD_LEVEL},
		{"Is Sinle Physics", NodeType::ACTION, NodeImplementation::IS_SINLE_PHYSICS},
		{"Disable Collision", NodeType::ACTION, NodeImplementation::DISABLE_COLLISION},
		{"Enable Physics", NodeType::ACTION, NodeImplementation::ENABLE_PHYSICS},
		{"Disable Physics", NodeType::ACTION, NodeImplementation::DISABLE_PHYSICS},
		{"Enter Water", NodeType::ACTION, NodeImplementation::ENTER_WATER},
		{"Exit Water", NodeType::ACTION, NodeImplementation::EXIT_WATER},
		{"Hit Ground", NodeType::ACTION, NodeImplementation::HIT_GROUND},
		{"Not on Ground", NodeType::ACTION, NodeImplementation::NOT_ON_GROUND},
		{"Change Acceleration Direction", NodeType::ACTION, NodeImplementation::CHANGE_ACCEL_DIRECTION},
		{"Set Sinle Acceleration", NodeType::ACTION, NodeImplementation::SET_SINLE_ACCEL},
		{"Play Random Sound", NodeType::ACTION, NodeImplementation::PLAY_RANDOM_SOUND},
		{"Lock Position", NodeType::ACTION, NodeImplementation::LOCK_POSITION},
		{"Unlock Position", NodeType::ACTION, NodeImplementation::UNLOCK_POSITION}

	};
	/* ----------------- Logic End ---------------- */


	/* ----------------- Component Entry ---------------- */
	struct ComponentEntry {
		std::string name;
		std::function<void(GameObject*)>addComp; // func to add component (hlp3 is now my fav module)
		std::function<bool(const GameObject*)>hasComp; // func to check if has component
		std::function<void(GameObject*)> removeComp; // func to remove component

		// for undo/redo
		std::function<std::string(GameObject*)> serializeComp;   // component -> JSON string
		std::function<void(GameObject*, std::string)> deserializeComp; // JSON string -> component
	};
	/* ----------------- Component End ---------------- */

	inline MenuBarState menuBarState;
	inline ObjSelectionState objSelectionState;
	inline ShapeControlState circleInput;
	inline ShapeControlState rectangleInput;
	inline SceneWindowState sceneWindowState;
	inline ScriptState scriptState;
	inline AssetBrowserState assetBrowserState;
	inline SceneState sceneState;
	inline PopupState popupState;
	inline GizmoState gizmoState;
	inline VisualLogicState visualLogicState;


	/* ----------------- Component Registry ---------------- */
	// add useful func here to check/add components for Editor Inspector
	// do not add transform component here, every obj should by default has a transform alr
	inline static std::vector<ComponentEntry> componentRegistry{
		{"Render",
			[](GameObject* obj) {
			Render* r = obj->addComponent<Render>();
			r->modelRef = renderer::models[static_cast<int>(shape::square)];
			DebugLog::addMessage("Added Render component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Render>(); },
			[](GameObject* obj) { obj->removeComponent<Render>(); },
			[](GameObject* obj) { return JsonIO::serializeRender(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeRender(obj, data); }
		},

		{"Collision",
			[] (GameObject* obj) {
			CollisionInfo* c = obj->addComponent<CollisionInfo>();

			Transform* t = obj->getComponent<Transform>();
			if (t) {
				c->colliderSize.x = t->scaleX;
				c->colliderSize.y = t->scaleY;
			}

			Render* r = obj->getComponent<Render>();
			if (r) {
				c->colliderType = r->modelRef.shape;
			}

			DebugLog::addMessage("Added Collision component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<CollisionInfo>(); },
			[](GameObject* obj) { obj->removeComponent<CollisionInfo>(); },
			[](GameObject* obj) { return JsonIO::serializeCollision(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeCollision(obj, data); }
		},

		{"Physics",
			[] (GameObject* obj) {
			obj->addComponent<Physics>();
			DebugLog::addMessage("Added Physics component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Physics>(); },
			[](GameObject* obj) { obj->removeComponent<Physics>(); },
			[](GameObject* obj) { return JsonIO::serializePhysics(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializePhysics(obj, data); }
		},

		{"Input Control",
			[](GameObject* obj) {
			obj->addComponent<Input>();
			DebugLog::addMessage("Added Input component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Input>(); },
			[](GameObject* obj) { obj->removeComponent<Input>(); },
			[](GameObject* obj) { return JsonIO::serializeInput(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeInput(obj, data); }
		},

		{"State Machine",
			[](GameObject* obj) {
			obj->addComponent<StateMachine>();
			DebugLog::addMessage("Added State Machine for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<StateMachine>(); },
			[](GameObject* obj) { obj->removeComponent<StateMachine>(); },
			[](GameObject* obj) { return JsonIO::serializeStateMachine(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeStateMachine(obj, data); }
		},
		{"Font",
			[](GameObject* obj) {
			obj->addComponent<FontComponent>();
			obj->getComponent<FontComponent>()->mdl = Font::fontMdls[0];
			DebugLog::addMessage("Added Font component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<FontComponent>(); },
			[](GameObject* obj) { obj->removeComponent<FontComponent>(); },
			[](GameObject* obj) { return JsonIO::serializeFont(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeFont(obj, data); }
		},
		{"Audio",
			[](GameObject* obj) {
			obj->addComponent<AudioComponent>();
			DebugLog::addMessage("Added Audio component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<AudioComponent>(); },
			[](GameObject* obj) { obj->removeComponent<AudioComponent>(); },
			[](GameObject* obj) { return JsonIO::serializeAudio(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeAudio(obj, data); }
		},
		{"Tile Map",
			[](GameObject* obj) {
			obj->addComponent<TileMap>();
			DebugLog::addMessage("Added Tile Map component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<TileMap>(); },
			[](GameObject* obj) { 
				gizmoState.showGrid = false;
				obj->removeComponent<TileMap>(); },
			[](GameObject* obj) { return JsonIO::serializeTileMap(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeTileMap(obj, data); }
		},
		{"Lua Script",
			[](GameObject* obj) {
			obj->addComponent<LuaScript>();
			DebugLog::addMessage("Added Lua Script component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<LuaScript>(); },
			[](GameObject* obj) { obj->removeComponent<LuaScript>(); },
			[](GameObject* obj) { return JsonIO::serializeLuaScript(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeLuaScript(obj, data); }
		},
		{"Particle",
			[](GameObject* obj) {
			obj->addComponent<Particle>();
			DebugLog::addMessage("Added Particle component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Particle>(); },
			[](GameObject* obj) { obj->removeComponent<Particle>(); },
			[](GameObject* obj) { return JsonIO::serializeParticle(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeParticle(obj, data); }
		},
		{"Visual Logic",
			[](GameObject* obj) {
			obj->addComponent<VisualLC>();
			DebugLog::addMessage("Added Virtual Logic component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<VisualLC>(); },
			[](GameObject* obj) { obj->removeComponent<VisualLC>(); },
			[](GameObject* obj) { return JsonIO::serializeVisualLC(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeVisualLC(obj, data); }
		},
		{ "Animation",
			[](GameObject* obj) {
			obj->addComponent<Animation>();
			DebugLog::addMessage("Added Virtual Logic component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Animation>(); },
			[](GameObject* obj) { obj->removeComponent<Animation>(); },
			[](GameObject* obj) { return JsonIO::serializeAnimation(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeAnimation(obj, data); }
		},
		{ "Water Body",
			[](GameObject* obj) { obj->addComponent<WaterBody>(); },
			[](const GameObject* obj) { return obj->hasComponent<WaterBody>(); },
			[](GameObject* obj) { obj->removeComponent<WaterBody>(); },
			[](GameObject* obj) { return JsonIO::serializeWaterBody(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeWaterBody(obj, data); }
		},
		{ "Weapon",
			[](GameObject* obj) { obj->addComponent<WeaponComponent>(); },
			[](const GameObject* obj) { return obj->hasComponent<WeaponComponent>(); },
			[](GameObject* obj) { obj->removeComponent<WeaponComponent>(); },
			[](GameObject* obj) { return JsonIO::serializeWeapon(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeWeapon(obj, data); }
		},
		{ "Button",
			[](GameObject* obj) { obj->addComponent<Button>(); },
			[](const GameObject* obj) { return obj->hasComponent<Button>(); },
			[](GameObject* obj) { obj->removeComponent<Button>(); },
			[](GameObject* obj) { return JsonIO::serializeButton(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeButton(obj, data); }
		},
		{ "Camera",
			[](GameObject* obj) {
			obj->addComponent<camera>();
			hasCam = true;
			DebugLog::addMessage("Added Camera component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<camera>(); },
			[](GameObject* obj) { obj->removeComponent<camera>(); hasCam = false; },
			[](GameObject* obj) { return JsonIO::serializeCamera(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeCamera(obj, data); }
		},
		{ "Video",
			[](GameObject* obj) {
			obj->addComponent<Video>();
			DebugLog::addMessage("Added Video component for " + obj->getObjectName());
			},
			[](const GameObject* obj) { return obj->hasComponent<Video>(); },
			[](GameObject* obj) { obj->removeComponent<Video>(); },
			[](GameObject* obj) { return JsonIO::serializeVideo(obj); },
			[](GameObject* obj, std::string data) { JsonIO::deserializeVideo(obj, data); }
		}
	};

	// find component entry
	inline const ComponentEntry* findEntry(const std::string& name) {
		for (const ComponentEntry& e : componentRegistry)
			if (e.name == name) return &e;
		return nullptr;
	}
	/* ----------------- Component End ---------------- */
}
//#endif