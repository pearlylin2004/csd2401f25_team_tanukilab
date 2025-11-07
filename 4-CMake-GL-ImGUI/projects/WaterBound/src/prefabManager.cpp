/* Start Header ************************************************************************/
/*!
\file       prefabManager.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 80% of the file
			Varick, v.teo, 2403417
			- 20% of the file
\par        cheeqing.tan@digipen.edu
\par		v.teo@digipen.edu
\date       November, 07th, 2025
\brief      A class that load and manage the prefabs by maintaining a prefab registry.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/assets"
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

static const char* playerStateToStr(PlayerState state) {
	switch (state) {
	case PlayerState::Idle:     return "Idle";
	case PlayerState::Walking:  return "Walking";
	case PlayerState::Jumping:  return "Jumping";
	case PlayerState::Falling:  return "Falling";
	case PlayerState::Shooting: return "Shooting";
	default:                    return "Idle";
	}
}

PrefabManager& PrefabManager::Instance() {
	static PrefabManager instance;
	return instance;
}

void PrefabManager::loadPrefabRegistry() {
	rapidjson::Document doc;
	std::string err;

	std::string filename = std::string(RUNTIME_DIR_R) + registryFile;

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

void PrefabManager::savePrefabRegistry() {
	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	//rapidjson::Value prefabs(rapidjson::kArrayType);

	// version number (same as loader)
	doc.AddMember("version", 1, a);

	// Create the prefabIndex array
	rapidjson::Value prefabArray(rapidjson::kArrayType);

	for (const auto& [id, prefab] : m_prefabRegistry) {
		rapidjson::Value prefabObj(rapidjson::kObjectType);

		prefabObj.AddMember("id", rapidjson::Value(id.c_str(), a), a);
		prefabObj.AddMember("name", rapidjson::Value(prefab.name.c_str(), a), a);
		prefabObj.AddMember("path", rapidjson::Value(prefab.path.c_str(), a), a);

		prefabArray.PushBack(prefabObj, a);
	}

	doc.AddMember("prefabIndex", prefabArray, a);

	// write to run time file
	std::string runtime = std::string(RUNTIME_DIR_R) + registryFile;
	std::string err;
	if (!JsonIO::WriteDocumentToFile(runtime, doc, /*pretty=*/true, &err)) {
		std::cerr << "Registry save failed: " << err << "\n";
		return;
	}

	// write to source (project dir)
	std::string dir = std::string(SOURCE_DIR_R) + registryFile;
	if (!JsonIO::WriteDocumentToFile(dir, doc, /*pretty=*/true, &err)) {
		std::cerr << "Prefab save failed (source): " << err << "\n";
		return;
	}
	else {
		std::cout << "Prefab saved (source JSON): " << dir << "\n";
	}

	std::cout << "Prefab registry saved at " << registryFile << "\n";
}

//std::string& PrefabManager::getPrefabPath(const std::string& prefabID) {
//	return m_prefabRegistry[prefabID].path;
//}

const std::string& PrefabManager::getPrefabPath(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).path;
}

bool PrefabManager::setPrefabPath(const std::string& prefabID, const std::string& newName) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		std::cout << "Prefab ID " << prefabID << "not found.\n";
		return false;
	}

	std::filesystem::path path{ newName };
	std::string newNameWPath = path.filename().string();

	iterator->second.path = "/Prefab/" + newNameWPath;
	savePrefabRegistry();
	std::cout << "[Debug] Updated prefab path for ID " << prefabID << " -> " << newNameWPath << "\n";
	return true;
}

std::string PrefabManager::getPrefabFilename(const std::string& prefabID) const {
	auto it = m_prefabRegistry.find(prefabID);
	if (it == m_prefabRegistry.end()) {
		std::cerr << "[Error] Prefab ID '" << prefabID << "' not found.\n";
		return {};
	}

	// extract filename from path
	std::filesystem::path fullPath = it->second.path;
	return fullPath.filename().string();  // e.g. "platform.json"
}


std::string& PrefabManager::getPrefabName(const std::string& prefabID) {
	return m_prefabRegistry[prefabID].name;
}

const std::string& PrefabManager::getPrefabName(const std::string& prefabID) const {
	return m_prefabRegistry.at(prefabID).name;
}

std::string PrefabManager::findPrefabIDByFilename(const std::filesystem::path& filename) {
	std::filesystem::path filenameOnly{ filename };
	std::string f{ filenameOnly.filename().string() };

	for (const auto& [id, info] : m_prefabRegistry) {
		std::filesystem::path path{ info.path };
		std::string prefabFile{ path.filename().string() }; // extract only the filename of the 

		if (prefabFile == f) {
			return id;
		}
	}

	return "";
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

GameObject* PrefabManager::instantiate(GameObject* go, GameObjectManager& manager) {
	const std::string& id = go->getObjectPrefabID();
	auto iterator = m_prefabRegistry.find(id);
	if (iterator == m_prefabRegistry.end() || id.empty()) {
		std::cerr << "PrefabManager::instantiate() failed � prefab ID not found: " << id << "\n";
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

	if (doc.HasMember("layer") && doc["layer"].IsInt()) {
		int layerID = doc["layer"].GetInt();
		manager.assignObjectToLayer(go, layerID);
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
			r->texChanged = false;   // lazy GPU load later
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

std::unique_ptr<GameObject> PrefabManager::createTempPrefabObj(const std::string& prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		std::cerr << "PrefabManager::createTempPrefabObject: failed � prefab ID not found: " << prefabID << "\n";
		return nullptr;
	}

	const std::string& path = std::string(RUNTIME_DIR_R) + iterator->second.path;

	rapidjson::Document doc;
	auto cacheIterator = m_prefabCache.find(prefabID);
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

		m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());
	}

	if (!doc.HasMember("components") || !doc["components"].IsObject()) {
		std::cerr << "Prefab file '" << path << "' missing COMPONENT object.\n";
		return nullptr;
	}

	std::unique_ptr<GameObject> tempGO = GameObjectManager::createTempGameObject(iterator->second.name);
	tempGO->getObjectPrefabID() = prefabID;

	const auto& comps = doc["components"];

	// ---- Transform ----
	if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
		const auto& jt = comps["Transform"];
		auto* t = tempGO->addComponent<Transform>();

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
		auto* r = tempGO->addComponent<Render>();

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


	}

	// ---- Input (presence only) ----
	if (comps.HasMember("Input") && comps["Input"].IsObject()) {
		tempGO->addComponent<Input>(1.f);
	}

	// ---- Physics ----
	if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
		const auto& jp = comps["Physics"];
		auto* p = tempGO->addComponent<Physics>();
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
		tempGO->addComponent<CollisionInfo>();
	}

	// ---- Animation ----
	if (comps.HasMember("Animation") && comps["Animation"].IsObject()) {
		const auto& ja = comps["Animation"];
		auto* a = tempGO->addComponent<Animation>();
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
		auto* sm = tempGO->addComponent<StateMachine>();

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
	return tempGO;
}

bool PrefabManager::savePrefab(const GameObject* prefabObj) {
	if (!prefabObj) return false;

	const std::string& prefabID = prefabObj->getObjectPrefabID();
	if (prefabID.empty()) return false;

	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// original prefab
	const rapidjson::Document* prefabDoc{ getPrefabJson(prefabID) };

	// prefab id
	doc.AddMember("id", rapidjson::Value(prefabID.c_str(), a), a);
	// name
	doc.AddMember("name", rapidjson::Value(prefabObj->getObjectName().c_str(), a), a);

	int layerID = prefabObj->getLayer();
	if (layerID != -1 && layerID != 1) {  // Only save if not default game layer
		doc.AddMember("layer", layerID, a);
	}

	// objects array
	//rapidjson::Value objects(rapidjson::kArrayType);
	// components container
	rapidjson::Value comps(rapidjson::kObjectType);

	/* ------------ Useful lambda for comparing later ------------ */
	// obj referencing prefab -> if new data added for scene, will override prefab data
	// compare 2 float
	auto differsFloat = [](float a, float b) { return std::fabs(a - b) > 1e-6f; };
	// compare 2 int
	auto differsInt = [](int a, int b) { return a != b; };
	// compare 2 boolean
	auto differsBool = [](bool a, bool b) { return a != b; };

	//// ---- Transform ----
	if (auto t = prefabObj->getComponent<Transform>()) {
		rapidjson::Value jt(rapidjson::kObjectType);
		rapidjson::Value pos(rapidjson::kArrayType);
		pos.PushBack(t->x, a).PushBack(t->y, a).PushBack(t->z, a);
		jt.AddMember("pos", pos, a);
		jt.AddMember("rotation", t->rotation, a);
		rapidjson::Value sc(rapidjson::kArrayType);
		sc.PushBack(t->scaleX, a).PushBack(t->scaleY, a).PushBack(t->scaleZ, a);
		jt.AddMember("scale", sc, a);
		comps.AddMember("Transform", jt, a);
	}

	//// ---- Render ----
	if (auto r = prefabObj->getComponent<Render>()) {
		rapidjson::Value jr(rapidjson::kObjectType);
		jr.AddMember("shape", rapidjson::Value(ShapeToStr(r->modelRef.shape), a), a);
		jr.AddMember("hasTex", r->hasTex ? 1 : 0, a);
		if (r->hasTex && !r->texFile.empty())
			jr.AddMember("texture", rapidjson::Value(r->texFile.c_str(), a), a);
		jr.AddMember("hasAnimation", r->hasAnimation ? 1 : 0, a);
		comps.AddMember("Render", jr, a);
	}

	//// ---- Physics ----
	if (auto p = prefabObj->getComponent<Physics>()) {
		rapidjson::Value jp(rapidjson::kObjectType);
		jp.AddMember("inWater", p->inWater ? 1 : 0, a);
		jp.AddMember("buoyancy", p->buoancy ? 1 : 0, a);
		comps.AddMember("Physics", jp, a);
	}

	//// ---- Animation ----
	if (auto an = prefabObj->getComponent<Animation>()) {
		rapidjson::Value ja(rapidjson::kObjectType);
		ja.AddMember("loop", an->loop ? 1 : 0, a);
		ja.AddMember("currentFrameColumn", an->currentFrameColumn, a);
		ja.AddMember("currentFrameRow", an->currentFrameRow, a);
		ja.AddMember("totalColumn", an->totalColumn, a);
		ja.AddMember("totalRow", an->totalRow, a);
		ja.AddMember("frameTime", an->frameTime, a);
		comps.AddMember("Animation", ja, a);
	}

	//// ---- StateMachine ----
	if (auto sm = prefabObj->getComponent<StateMachine>()) {
		rapidjson::Value js(rapidjson::kObjectType);
		js.AddMember("state", rapidjson::Value(playerStateToStr(sm->state), a), a);
		js.AddMember("facingRight", sm->facingRight ? 1 : 0, a);
		js.AddMember("stateTime", sm->stateTime, a);
		comps.AddMember("StateMachine", js, a);
	}

	doc.AddMember("components", comps, a);

	std::string path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
	std::string err;
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		std::cerr << "Prefab save failed (runtime): " << err << "\n";
		return false;
	}
	else {
		std::cout << "Prefab saved (runtime JSON): " << path << "\n";
	}

	path = std::string(SOURCE_DIR_R) + getPrefabPath(prefabID);
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		std::cerr << "Prefab save failed (source): " << err << "\n";
		return false;
	}
	else {
		std::cout << "Prefab saved (source JSON): " << path << "\n";
	}

	// update cache
	//m_prefabCache[prefabID].SetObject();
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	return true;
}

bool PrefabManager::createPrefabFromGameObj(GameObject* go, const std::string& name, bool makeRef) {
	if (!go) return false;

	std::string prefabID = generateUUID();

	// if Prefab folder not created yet, create one
	std::filesystem::path preFabDir = SOURCE_DIR_R + prefabFolder;
	if (!std::filesystem::exists(preFabDir)) {
		std::filesystem::create_directories(preFabDir);
	}

	// if name is not given, use the name of the game obj
	std::string prefabName = (name.empty()) ? go->getObjectName() : name;
	std::string filename = prefabFolder + "/" + prefabName + ".json";
	std::filesystem::path prefabPath = preFabDir / prefabName;

	auto pathExistsInRegistry = [this](const std::string& path) -> bool {
		for (const auto& [id, info] : m_prefabRegistry) {
			if (info.path == path) {
				return true;
			}
		}
		return false;
		};

	// Rename if path already registered
	if (pathExistsInRegistry(filename)) {
		int counter = 1;
		std::string newFilename;
		do {
			newFilename = prefabFolder + "/" + prefabName + "_" + std::to_string(counter) + ".json";
			counter++;
		} while (pathExistsInRegistry(newFilename));
		filename = newFilename;
	}

	PrefabInfo info;
	info.name = prefabName;
	info.path = filename;
	m_prefabRegistry[prefabID] = info;

	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// prefab id
	doc.AddMember("id", rapidjson::Value(prefabID.c_str(), a), a);
	// name
	doc.AddMember("name", rapidjson::Value(prefabName.c_str(), a), a);

	rapidjson::Value comps(rapidjson::kObjectType);

	// ---- Transform ----
	if (auto t = go->getComponent<Transform>()) {
		rapidjson::Value jt(rapidjson::kObjectType);

		rapidjson::Value pos(rapidjson::kArrayType);
		pos.PushBack(t->x, a).PushBack(t->y, a).PushBack(t->z, a);
		jt.AddMember("pos", pos, a);

		jt.AddMember("rotation", t->rotation, a);

		rapidjson::Value sc(rapidjson::kArrayType);
		sc.PushBack(t->scaleX, a).PushBack(t->scaleY, a).PushBack(t->scaleZ, a);
		jt.AddMember("scale", sc, a);

		comps.AddMember("Transform", jt, a);
	}

	// ---- Render ----
	if (auto r = go->getComponent<Render>()) {
		rapidjson::Value jr(rapidjson::kObjectType);

		jr.AddMember("shape", rapidjson::Value(ShapeToStr(r->modelRef.shape), a), a);

		jr.AddMember("hasTex", r->hasTex ? 1 : 0, a);

		if (r->hasTex && !r->texFile.empty())
			jr.AddMember("texture", rapidjson::Value(r->texFile.c_str(), a), a);

		jr.AddMember("hasAnimation", r->hasAnimation ? 1 : 0, a);

		comps.AddMember("Render", jr, a);
	}

	// ---- Physics ----
	if (auto p = go->getComponent<Physics>()) {
		rapidjson::Value jp(rapidjson::kObjectType);

		jp.AddMember("inWater", p->inWater ? 1 : 0, a);

		jp.AddMember("buoyancy", p->buoancy ? 1 : 0, a);

		comps.AddMember("Physics", jp, a);
	}

	// ---- Animation ----
	if (auto an = go->getComponent<Animation>()) {
		rapidjson::Value ja(rapidjson::kObjectType);

		ja.AddMember("loop", an->loop ? 1 : 0, a);

		ja.AddMember("currentFrameColumn", an->currentFrameColumn, a);

		ja.AddMember("currentFrameRow", an->currentFrameRow, a);

		ja.AddMember("totalColumn", an->totalColumn, a);

		ja.AddMember("totalRow", an->totalRow, a);

		ja.AddMember("frameTime", an->frameTime, a);

		comps.AddMember("Animation", ja, a);
	}

	// ---- StateMachine ----
	if (auto sm = go->getComponent<StateMachine>()) {
		rapidjson::Value js(rapidjson::kObjectType);

		js.AddMember("state", rapidjson::Value(playerStateToStr(sm->state), a), a);

		js.AddMember("facingRight", sm->facingRight ? 1 : 0, a);

		js.AddMember("stateTime", sm->stateTime, a);

		comps.AddMember("StateMachine", js, a);
	}

	doc.AddMember("components", comps, a);

	/* -------- Save new prefab to a new .json file ------- */
	std::string path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
	std::string err;
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		std::cerr << "Prefab save failed (runtime): " << err << "\n";
		return false;
	}
	else {
		std::cout << "Prefab saved (runtime JSON): " << path << "\n";
	}

	path = std::string(SOURCE_DIR_R) + getPrefabPath(prefabID);
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		std::cerr << "Prefab save failed (source): " << err << "\n";
		return false;
	}
	else {
		std::cout << "Prefab saved (source JSON): " << path << "\n";
	}
	/* -------- END ------- */

	// save in prefab registry file
	savePrefabRegistry();

	// set this game object to be reference to the created prefab
	if(makeRef) go->getObjectPrefabID() = prefabID;

	// update cache
	//m_prefabCache[prefabID].SetObject();
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	return true;
}

bool PrefabManager::deletePrefab(const std::string prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		std::cerr << "PrefabManager::deleteprefab: failed � prefab ID not found: " << prefabID << "\n";
		return false;
	}

	// remove prefab from prefab registry and prefab cache
	m_prefabRegistry.erase(iterator);
	m_prefabCache.erase(prefabID);

	// save updated prefab registry to file
	savePrefabRegistry();

	return true;
}

std::string PrefabManager::generateUUID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned> dis1(0, 15); // 0 - F
	std::uniform_int_distribution<unsigned> dis2(8, 11); // variant

	std::stringstream uuid;
	int i;
	for (i = 0; i < 8; ++i) uuid << std::hex << dis1(gen); // first 8 hex
	uuid << "-";
	for (i = 0; i < 4; ++i) uuid << std::hex << dis1(gen); // next 4 digits
	uuid << "-";
	uuid << 4; // version 4
	for (i = 0; i < 3; ++i) uuid << std::hex << dis1(gen); // next 3 digits
	uuid << "-";
	uuid << std::hex << dis2(gen); // variant
	for (i = 0; i < 3; ++i) uuid << std::hex << dis1(gen); // next 3 digits
	uuid << "-";
	for (i = 0; i < 12; ++i) uuid << std::hex << dis1(gen); // last 12 digits

	return uuid.str();
}