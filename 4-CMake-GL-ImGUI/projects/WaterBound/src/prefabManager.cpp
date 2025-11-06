/* Start Header ************************************************************************/
/*!
\file       input.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 15th, 2025
\brief      This source file implements the InputHandler class, which centralizes input
			handling for GLFW. It maintains arrays of current and previous states for
			keyboard keys and mouse buttons, enabling detection of triggered, released,
			held, and combo inputs.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./shaders"
#endif
#include "prefabManager.h"
#include "JsonIO.h"

// -- helpers --
namespace {
	const char* ShapeToStr(shape s) {
		switch (s) {
		case shape::square:   return "square";
		case shape::circle:   return "circle";
		case shape::triangle: return "triangle";
		default:              return "square";
		}
	}
	shape StrToShape(const std::string& s) {
		if (s == "square")   return shape::square;
		if (s == "circle")   return shape::circle;
		if (s == "triangle") return shape::triangle;
		return shape::square;
	}
	inline bool ends_with(const std::string& s, const char* suf) {
		const size_t n = std::strlen(suf);
		return s.size() >= n && 0 == s.compare(s.size() - n, n, suf);
	}
}

static PlayerState parsePlayerState(const std::string& s) {
	if (s == "Idle")     return PlayerState::Idle;
	if (s == "Walking")  return PlayerState::Walking;
	if (s == "Jumping")  return PlayerState::Jumping;
	if (s == "Falling")  return PlayerState::Falling;
	if (s == "Shooting") return PlayerState::Shooting;
	return PlayerState::Idle; // default/fallback
}

PrefabManager& PrefabManager::Instance() {
	static PrefabManager instance;
	return instance;
}

void PrefabManager::loadPrefabRegistry(const std::string& filename) {
	rapidjson::Document doc;
	std::string err;

	if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
		std::cerr << "Prefab registry load failed: " << err << "\n";
		return;
	}

	int version{ 1 };
	if (doc.HasMember("version") && doc["version"].IsInt())
		version = doc["version"].GetInt();
	if (version != 1)
		std::cerr << "Warning: unsupported prefab registry version " << version << std::endl;

	m_prefabRegistry.clear();

	if (!doc.HasMember("prefabIndex") || !doc["prefabIndex"].IsArray()) {
		std::cerr << "Prefab registry JSON missing PREFABINDEX array.\n";
		return;
	}

	for (auto& jPrefab : doc["prefabIndex"].GetArray()) {
		if (!jPrefab.IsObject()) continue;

		std::string id = (jPrefab.HasMember("id") && jPrefab["id"].IsString()) ? jPrefab["id"].GetString() : "";

		std::string name = (jPrefab.HasMember("name") && jPrefab["name"].IsString())
			? jPrefab["name"].GetString()
			: ("Prefab_" + std::to_string(m_prefabRegistry.size()));

		std::string path = (jPrefab.HasMember("path") && jPrefab["path"].IsString()) ? jPrefab["path"].GetString() : "";

		if (id.empty()) {
			std::cerr << "Skipping prefab " << name << " with missing id.\n";
		}

		m_prefabRegistry[id] = { name, path };
	}

	std::cout << "Prefab registry loaded: " << filename << " (" << m_prefabRegistry.size() << " prefabs)\n";
}

std::string& PrefabManager::getPrefabPath(const std::string& prefabID) {
	return m_prefabRegistry[prefabID].path;
}

const std::string& PrefabManager::getPrefabPath(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).path;
}

std::string& PrefabManager::getPrefabName(const std::string& prefabID) {
	return m_prefabRegistry[prefabID].name;
}

const std::string& PrefabManager::getPrefabName(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).name;
}

const rapidjson::Document* PrefabManager::getPrefabJson(const std::string& prefabID) const {
	auto iterator = m_prefabCache.find(prefabID);
	if (iterator != m_prefabCache.end()) {
		return &iterator->second;
	}
	return nullptr;
}

const rapidjson::Value* PrefabManager::getPrefabJsonComp(const rapidjson::Document* prefabDoc, const std::string& compName) const {
	// if prefab dont have components then return
	if (!prefabDoc || !prefabDoc->HasMember("components")) return nullptr;

	// if prefab dont have the given components
	const auto& comps = (*prefabDoc)["components"];
	if (!comps.HasMember(compName.c_str())) return nullptr;

	return &comps[compName.c_str()];
}

GameObject* PrefabManager::instantiate(GameObject* go, GameObjectManager&) {
	const std::string& id = go->getObjectPrefabID();
	auto iterator = m_prefabRegistry.find(id);
	if (iterator == m_prefabRegistry.end() || id.empty()) {
		std::cerr << "PrefabManager::instantiate() failed — prefab ID not found: " << id << "\n";
		return nullptr;
	}

	//const std::string& path = iterator->second.path;
	const std::string& path = std::string(RUNTIME_DIR_R) + iterator->second.path;

	rapidjson::Document doc;
	auto cacheIterator = m_prefabCache.find(id);
	if (cacheIterator != m_prefabCache.end()) {
		// if json is loaded previously, use it
		doc.CopyFrom(cacheIterator->second, doc.GetAllocator());
	}
	else {
		// if json never load before, read from disk
		std::string err;
		if (!JsonIO::ReadFileToDocument(path, doc, &err)) {
			std::cerr << "Failed to load prefab file '" << path << "': " << err << "\n";
			return nullptr;
		}

		m_prefabCache[id].CopyFrom(doc, m_prefabCache[id].GetAllocator());
	}

	if (!doc.HasMember("components") || !doc["components"].IsObject()) {
		std::cerr << "Prefab file '" << path << "' missing COMPONENT object.\n";
		return nullptr;
	}

	/* --------- Load Component ---------*/
	const auto& comps = doc["components"];

	// ---- Transform ----
	if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
		const auto& jt = comps["Transform"];
		auto* t = go->addComponent<Transform>();

		if (jt.HasMember("pos") && jt["pos"].IsArray() && jt["pos"].Size() == 3) {
			t->x = jt["pos"][0].GetFloat();
			t->y = jt["pos"][1].GetFloat();
			t->z = jt["pos"][2].GetFloat();
		}
		if (jt.HasMember("rotation") && jt["rotation"].IsNumber()) {
			t->rotation = jt["rotation"].GetFloat();
		}
		if (jt.HasMember("scale") && jt["scale"].IsArray() && jt["scale"].Size() == 3) {
			t->scaleX = jt["scale"][0].GetFloat();
			t->scaleY = jt["scale"][1].GetFloat();
			t->scaleZ = jt["scale"][2].GetFloat();
		}

		/*std::cout << "\n\nReading prefab...\n";
		std::cout << "Transform: x: " << t->x << ", y: " << t->y << ", z: " << t->z << std::endl;
		std::cout << "Rotation: " << t->rotation << std::endl;
		std::cout << "Scale: x: " << t->scaleX << ", y: " << t->scaleY << ", z: " << t->scaleZ << std::endl;*/
	}

	// ---- Render ----
	if (comps.HasMember("Render") && comps["Render"].IsObject()) {
		const auto& jr = comps["Render"];
		auto* r = go->addComponent<Render>();

		// shape
		std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
			? jr["shape"].GetString()
			: "square";
		r->modelRef = renderer::models[static_cast<int>(StrToShape(shp))];

		// hasTex / texture (accept bool or 0/1)
		if (jr.HasMember("hasTex")) {
			r->hasTex = jr["hasTex"].IsBool()
				? jr["hasTex"].GetBool()
				: (jr["hasTex"].GetInt() != 0);
		}
		else {
			r->hasTex = false;
		}

		if (r->hasTex && jr.HasMember("texture") && jr["texture"].IsString()) {
			r->texFile = jr["texture"].GetString();
			r->texHDL = 0;
			r->texChanged = true;   // lazy GPU load later
		}
		else {
			r->texFile.clear();
			r->texHDL = 0;
			r->texChanged = false;
		}

		if (jr.HasMember("hasAnimation")) {
			r->hasAnimation = jr["hasAnimation"].IsBool()
				? jr["hasAnimation"].GetBool()
				: (jr["hasAnimation"].GetInt() != 0);
		}
		else {
			r->hasAnimation = false;
		}

		if (jr.HasMember("clr") && jr["clr"].IsArray() && jr["clr"].Size() == 3)
		{
			r->clr.r = jr["clr"][0].GetFloat();
			r->clr.g = jr["clr"][1].GetFloat();
			r->clr.b = jr["clr"][2].GetFloat();
		}

	}

	// ---- Input (presence only) ----
	if (comps.HasMember("Input") && comps["Input"].IsObject()) {
		go->addComponent<Input>(1.f);
	}

	// ---- Physics ----
	if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
		const auto& jp = comps["Physics"];
		auto* p = go->addComponent<Physics>();
		if (jp.HasMember("inWater")) {
			p->inWater = jp["inWater"].IsBool()
				? jp["inWater"].GetBool()
				: (jp["inWater"].GetInt() != 0);
		}
		if (jp.HasMember("buoyancy")) {
			p->buoancy = jp["buoyancy"].IsBool()
				? jp["buoyancy"].GetBool()
				: (jp["buoyancy"].GetInt() != 0);
		}
	}

	// ---- CollisionInfo (presence only) ----
	if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
		go->addComponent<CollisionInfo>();
	}

	// ---- Animation ----
	if (comps.HasMember("Animation") && comps["Animation"].IsObject()) {
		const auto& ja = comps["Animation"];
		auto* a = go->addComponent<Animation>();
		if (ja.HasMember("loop")) {
			a->loop = ja["loop"].IsBool()
				? ja["loop"].GetBool()
				: (ja["loop"].GetInt() != 0);
		}
		if (ja.HasMember("currentFrameColumn") && ja["currentFrameColumn"].IsInt())
			a->currentFrameColumn = ja["currentFrameColumn"].GetInt();
		if (ja.HasMember("currentFrameRow") && ja["currentFrameRow"].IsInt())
			a->currentFrameRow = ja["currentFrameRow"].GetInt();
		if (ja.HasMember("totalColumn") && ja["totalColumn"].IsInt())
			a->totalColumn = ja["totalColumn"].GetInt();
		if (ja.HasMember("totalRow") && ja["totalRow"].IsInt())
			a->totalRow = ja["totalRow"].GetInt();
		if (ja.HasMember("frameTime") && ja["frameTime"].IsNumber())
			a->frameTime = ja["frameTime"].GetFloat();
	}
	
	// ---- State Machine ----
	if (comps.HasMember("StateMachine") && comps["StateMachine"].IsObject()) {
		const auto& smj = comps["StateMachine"];
		auto* sm = go->addComponent<StateMachine>();

		if (smj.HasMember("state") && smj["state"].IsString())
			sm->state = parsePlayerState(smj["state"].GetString());

		if (smj.HasMember("facingRight")) {
			if (smj["facingRight"].IsBool())       sm->facingRight = smj["facingRight"].GetBool();
			else if (smj["facingRight"].IsInt())   sm->facingRight = (smj["facingRight"].GetInt() != 0);
		}

		if (smj.HasMember("stateTime") && smj["stateTime"].IsNumber())
			sm->stateTime = static_cast<float>(smj["stateTime"].GetDouble());

		/*std::cout << "\n\nReading from prefab now...\n";
		std::cout << "State: " << sm->facingRight << std::endl;
		std::cout << "State Time: " << sm->stateTime << std::endl;*/
	}

	// std::cout << "Instantiated prefab '" << "' (" << id << ")\n";
	return go;
}