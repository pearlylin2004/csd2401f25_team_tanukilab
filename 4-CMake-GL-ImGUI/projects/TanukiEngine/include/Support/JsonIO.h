/* Start Header ************************************************************************/
/*!
\file		JsonIO.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declarations for JSON loading/saving utilities for the WaterBound engine.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>     
#include <filesystem>
#include <iostream>
#include <array>
#include <vector>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>

#pragma warning(push, 0)
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#pragma warning(pop)

#include "GameObjectManager.h"
#include "Component.h"
#include "renderer.h"

/*!
 * \namespace Paths
 * \brief Provides OS-aware helpers to locate engine directories relative
 *        to the executable (EXE) directory.
 *
 * These utilities ensure that the engine can load configs, shaders and scenes
 * correctly regardless of where the executable is deployed.
 */
namespace Paths {
	std::filesystem::path exe_dir();
	std::filesystem::path shaders();
	std::filesystem::path scenes();
	std::filesystem::path config();
	std::filesystem::path scripts();
	void ensure_dir(const std::filesystem::path& p);
}

/*!
 * \namespace JsonIO
 * \brief Utilities for JSON reading, writing, conversion, and component serialization.
 *
 * This namespace centralizes all JSON-related operations in the engine, keeping
 * core systems independent of RapidJSON internals. It provides:
 *
 * 1. **Path Helpers**
 *    - Compute source and runtime paths for scenes and project directories.
 *      Functions: `runtimeScenePath`, `sourceScenePath`, `projectSourceRoot`.
 *
 * 2. **Enum/String Conversion Helpers**
 *    - Convert enums to strings and back for serialization and editor display.
 *      Functions: `ShapeToStr`, `StrToShape`, `CollisionResponseModeToStr`,
 *                 `StrToCollisionResponseMode`, `ButtonActionToStr`,
 *                 `StrToButtonAction`, `parsePlayerState`,
 *                 `stringToNodeImpl`, `nodeImplToString`,
 *                 `stringToGLFWKey`.
 *
 * 3. **JSON File I/O Helpers**
 *    - Read and write JSON documents safely, sync runtime scenes, perform
 *      atomic file moves, and access JSON values.
 *      Functions: `ReadFileToDocument`, `WriteDocumentToFile`, `AtomicMove`,
 *                 `syncSceneToRuntime`, `GetObj`, `GetString`, `GetBool`,
 *                 `GetInt`, `GetFloat`, `GetIntOr`, `GetFloatOr`, `GetBoolOr`.
 *
 * 4. **Array and Vector Helpers**
 *    - Read/write fixed-size arrays and glm::vectors to/from JSON.
 *      Functions: `GetArray2f`, `GetArray3f`, `GetArray4f`,
 *                 `PutArray2f`, `PutArray3f`, `PutArray4f`,
 *                 `ReadVec2/3/4`, `WriteVec2/3/4`.
 *
 * 5. **Color Helpers**
 *    - Read/write colors as either float arrays or glm::vec4 structures.
 *      Functions: `ReadColor`, `WriteColor`.
 *
 * 6. **Collision Tag Helpers**
 *    - Load and save global collision tags used for physics and gameplay.
 *      Functions: `LoadCollisionTags`, `SaveCollisionTags`.
 *
 * 7. **Component Serialization/Deserialization**
 *    - Convert `GameObject` components to JSON strings and back, enabling
 *      undo/redo functionality, editor persistence, and runtime loading.
 *    - Each component type has a paired serialize/deserialize function:
 *        - Transform: `serializeTransform` / `deserializeTransform`
 *        - Render: `serializeRender` / `deserializeRender`
 *        - Particle: `serializeParticle` / `deserializeParticle`
 *        - Physics: `serializePhysics` / `deserializePhysics`
 *        - Collision: `serializeCollision` / `deserializeCollision`
 *        - Input: `serializeInput` / `deserializeInput`
 *        - Font: `serializeFont` / `deserializeFont`
 *        - Audio: `serializeAudio` / `deserializeAudio`
 *        - Animation: `serializeAnimation` / `deserializeAnimation`
 *        - StateMachine: `serializeStateMachine` / `deserializeStateMachine`
 *        - TileMap: `serializeTileMap` / `deserializeTileMap`
 *        - LuaScript: `serializeLuaScript` / `deserializeLuaScript`
 *        - Visual Logic Controller: `serializeVisualLC` / `deserializeVisualLC`
 *        - Camera: `serializeCamera` / `deserializeCamera`
 *        - Weapon: `serializeWeapon` / `deserializeWeapon`
 *        - WaterBody: `serializeWaterBody` / `deserializeWaterBody`
 *        - Button: `serializeButton` / `deserializeButton`
 *        - Video: `serializeVideo` / `deserializeVideo`
 *    - Function: `serializeGameObj` / `deserializeGameObj` handle full
 *      `GameObject` serialization including all its components.
 *
 * 8. **Visual Logic Helpers**
 *    - Save/load node-based visual logic graphs for editor tools.
 *      Functions: `saveVisualLogic`, `loadVisualLogic`.
 *
 * 9. **Layer Serialization Helpers**
 *    - Serialize/deserialize layer references for objects in a scene.
 *      Functions: `SerializeLayerRef`, `DeserializeLayerRef`.
 *
 * Overall, `JsonIO` provides a **consistent interface** for converting
 * engine objects and data to JSON and back, ensuring safe file operations,
 * editor persistence, and runtime scene management.
 */
namespace JsonIO {

	// -----------------------
	// Path helpers
	// -----------------------
	std::string runtimeScenePath(const std::string& name);
	std::string sourceScenePath(const std::string& name);
	std::string projectSourceRoot();

	// -----------------------
	// Enum/String conversions
	// -----------------------
	// ---- Shape string helpers ----
	const char* ShapeToStr(shape s);
	shape StrToShape(const std::string& s);

	// ---- Collision Response Mode helper
	const char* CollisionResponseModeToStr(CollisionResponseMode mode);
	CollisionResponseMode StrToCollisionResponseMode(const std::string& s);

	// ---- Button helper
	const char* ButtonActionToStr(ButtonAction action);
	ButtonAction StrToButtonAction(const std::string& s);

	// ---- Player State helpers ----
	PlayerState parsePlayerState(const std::string& s);

	// ---- Node Implementation helpers ----
	NodeImplementation stringToNodeImpl(const std::string& str);
	std::string nodeImplToString(NodeImplementation impl);

	// ---- GLFW key helpers ----
	int stringToGLFWKey(const std::string& keyStr);


	// -----------------------
	// JSON file I/O helpers
	// -----------------------
	bool syncSceneToRuntime(const std::string& name);
	bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err = nullptr);
	bool ReadFileToDocument(const std::string& path, rapidjson::Document& doc, std::string* err = nullptr);
	bool WriteDocumentToFile(const std::string& path, rapidjson::Document& doc, bool pretty = true, std::string* err = nullptr);

	const rapidjson::Value* GetObj(const rapidjson::Value& parent, const char* key);
	bool GetString(const rapidjson::Value& obj, const char* key, std::string& out);
	bool GetBool(const rapidjson::Value& obj, const char* key, bool& out);
	bool GetInt(const rapidjson::Value& obj, const char* key, int& out);
	bool GetFloat(const rapidjson::Value& obj, const char* key, float& out);
	inline int   GetIntOr(const rapidjson::Value& obj, const char* key, int   def) { int v; return GetInt(obj, key, v) ? v : def; }
	inline float GetFloatOr(const rapidjson::Value& obj, const char* key, float def) { float v; return GetFloat(obj, key, v) ? v : def; }
	inline bool  GetBoolOr(const rapidjson::Value& obj, const char* key, bool  def) { bool v; return GetBool(obj, key, v) ? v : def; }

	// -----------------------
	// Array helpers (fixed-size and glm vectors)
	// -----------------------
	bool GetArray2f(const rapidjson::Value& obj, const char* key, std::array<float, 2>& out);
	bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out);
	bool GetArray4f(const rapidjson::Value& obj, const char* key, std::array<float, 4>& out);
	void PutArray2f(rapidjson::Value& obj, const char* key, const std::array<float, 2>& v, rapidjson::Document::AllocatorType& a);
	void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a);
	void PutArray4f(rapidjson::Value& obj, const char* key, const std::array<float, 4>& v, rapidjson::Document::AllocatorType& a);

	// ---------- glm::vec3 helpers ----------
	bool ReadVec2(const rapidjson::Value& obj, const char* key, glm::vec2& out);
	bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out);
	bool ReadVec4(const rapidjson::Value& obj, const char* key, glm::vec4& out);
	void WriteVec2(rapidjson::Value& obj, const char* key, const glm::vec2& v, rapidjson::Document::AllocatorType& a);
	void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a);
	void WriteVec4(rapidjson::Value& obj, const char* key, const glm::vec4& v, rapidjson::Document::AllocatorType& a);

	bool ReadVec2(const rapidjson::Value& obj, const char* key, float& x, float& y);
	bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z);
	bool ReadVec4(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z, float& w);
	void WriteVec2(rapidjson::Value& obj, const char* key, float x, float y, rapidjson::Document::AllocatorType& a);
	void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a);
	void WriteVec4(rapidjson::Value& obj, const char* key, float x, float y, float z, float w, rapidjson::Document::AllocatorType& a);

	// -----------------------
	// Color helpers
	// -----------------------
	// Read a 4-component float color array [r,g,b,a] from JSON
	bool ReadColor(const rapidjson::Value& obj,
		const char* key,
		float& r, float& g, float& b, float& alpha);

	// Write a 4-component float color array [r,g,b,a] to JSON
	void WriteColor(rapidjson::Value& obj,
		const char* key,
		float r, float g, float b, float alpha,
		rapidjson::Document::AllocatorType& alloc);

	// Read a glm::vec4 color (r,g,b,a) from JSON
	bool ReadColor(const rapidjson::Value& obj,
		const char* key,
		glm::vec4& out);

	// Write a glm::vec4 color (r,g,b,a) to JSON
	void WriteColor(rapidjson::Value& obj,
		const char* key,
		const glm::vec4& v,
		rapidjson::Document::AllocatorType& alloc);


	// -----------------------
	// Collision tag helpers
	// -----------------------
	bool LoadCollisionTags(const std::string& filepath, std::vector<std::string>& outTags, std::string* err = nullptr);
	bool SaveCollisionTags(const std::string& filepath, const std::vector<std::string>& tags, std::string* err = nullptr);

	// ---- for undo/redo in editor ----
	// -----------------------
	// Component-level serialization for GameObjects
	// -----------------------
	std::string serializeTransform(GameObject* obj);
	void deserializeTransform(GameObject* obj, const std::string& json);

	std::string serializeRender(GameObject* obj);
	void deserializeRender(GameObject* obj, const std::string& json);

	std::string serializeParticle(GameObject* obj);
	void deserializeParticle(GameObject* obj, const std::string& json);

	std::string serializePhysics(GameObject* obj);
	void deserializePhysics(GameObject* obj, const std::string& json);

	std::string serializeCollision(GameObject* obj);
	void deserializeCollision(GameObject* obj, const std::string& json);

	std::string serializeInput(GameObject* obj);
	void deserializeInput(GameObject* obj, const std::string& json);

	std::string serializeFont(GameObject* obj);
	void deserializeFont(GameObject* obj, const std::string& json);

	std::string serializeAudio(GameObject* obj);
	void deserializeAudio(GameObject* obj, const std::string& json);

	std::string serializeAnimation(GameObject* obj);
	void deserializeAnimation(GameObject* obj, const std::string& json);

	std::string serializeStateMachine(GameObject* obj);
	void deserializeStateMachine(GameObject* obj, const std::string& json);

	std::string serializeTileMap(GameObject* obj);
	void deserializeTileMap(GameObject* obj, const std::string& json);
		
	std::string serializeLuaScript(GameObject* obj);
	void deserializeLuaScript(GameObject* obj, const std::string& json);

	std::string serializeVisualLC(GameObject* obj);
	void deserializeVisualLC(GameObject* obj, const std::string& json);

	std::string serializeCamera(GameObject* obj);
	void deserializeCamera(GameObject* obj, const std::string& json);
	
	std::string serializeWeapon(GameObject* obj);
	void deserializeWeapon(GameObject* obj, const std::string& json);

	std::string serializeWaterBody(GameObject* obj);
	void deserializeWaterBody(GameObject* obj, const std::string& json);

	std::string serializeButton(GameObject* obj);
	void deserializeButton(GameObject* obj, const std::string& json);

	std::string serializeVideo(GameObject* obj);
	void deserializeVideo(GameObject* obj, const std::string& json);


	std::string serializeGameObj(GameObject* obj);
	GameObject* deserializeGameObj(GameObjectManager& manager, const std::string& jsonStr);

	// -----------------------
	// Visual logic (editor) helpers
	// -----------------------
	bool saveVisualLogic(const std::string& filepath, const std::vector<LogicNode>& nodes, const std::vector<LogicConnection>& connections, std::string* err = nullptr);
	bool loadVisualLogic(const std::string& filepath, std::vector<LogicNode>& nodes, std::vector<LogicConnection>& connections, std::string* err = nullptr);

	// ---- END ----

	// -----------------------
	// Layer serialization helpers
	// -----------------------
	void SerializeLayerRef(rapidjson::Value& parent, int layerID, 
	                       const LayerManager& lm, 
	                       rapidjson::Document::AllocatorType& a);
	
	int DeserializeLayerRef(const rapidjson::Value& parent, 
	                        LayerManager& lm, 
	                        int defaultLayer = 1);
}
