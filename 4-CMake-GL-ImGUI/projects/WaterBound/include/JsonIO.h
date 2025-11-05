//JsonIO.h
#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>     
#include <filesystem>
#include <iostream>
#include <array>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>


#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>

#include "GameObject.h"
#include "renderer.h"
#include "Component.h"



namespace Paths {
	std::filesystem::path exe_dir();
	std::filesystem::path shaders();
	std::filesystem::path scenes();
	std::filesystem::path config();
	std::filesystem::path scripts();
	void ensure_dir(const std::filesystem::path& p);
}
namespace JsonIO {

	// ---- Path helpers ----
	std::string runtimeScenePath(const std::string& name);
	std::string sourceScenePath(const std::string& name);
	std::string projectSourceRoot();

	// ---- Shape string helpers ----
	const char* ShapeToStr(shape s);
	shape StrToShape(const std::string& s);

	// Player state helpers
	PlayerState parsePlayerState(const std::string& s);
	const char* playerStateToStr(PlayerState state);

	// ---- JSON file IO helpers ----
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

	// ---------- Fixed-size array helpers (3 components) ----------
	bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out);
	void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a);

	// ---------- glm::vec3 helpers ----------
	bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out);
	void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a);


	bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z);
	void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a);

	// ---- RGBA (Color) helpers ----

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

}
