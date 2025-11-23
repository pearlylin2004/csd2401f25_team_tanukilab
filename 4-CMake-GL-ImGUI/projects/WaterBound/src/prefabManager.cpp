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
#define SOURCE_DIR_R "../projects/WaterBound/assets"
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
		DebugLog::addMessage("Prefab registry load failed: " + err);
		return;
	}

	int version{ 1 };
	if (doc.HasMember("version") && doc["version"].IsInt())
		version = doc["version"].GetInt();
	if (version != 1)
		DebugLog::addMessage("Warning: unsupported prefab registry version " + version);

	m_prefabRegistry.clear();

	if (!doc.HasMember("prefabIndex") || !doc["prefabIndex"].IsArray()) {
		DebugLog::addMessage("Prefab registry JSON missing PREFABINDEX array.");
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
			DebugLog::addMessage("Skipping prefab " + name + " with missing id.");
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
		DebugLog::addMessage("Registry save failed: " + err);
		return;
	}

	// write to source (project dir)
	std::string dir = std::string(SOURCE_DIR_R) + registryFile;
	if (!JsonIO::WriteDocumentToFile(dir, doc, /*pretty=*/true, &err)) {
		DebugLog::addMessage("Prefab save failed (source): " + err);
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
		DebugLog::addMessage("Prefab ID " + prefabID + "not found.");
		return false;
	}

	std::filesystem::path path{ newName };
	std::string newNameWPath = path.filename().string();

	iterator->second.path = "/Prefab/" + newNameWPath;
	savePrefabRegistry();
	DebugLog::addMessage("Updated prefab path for ID " + prefabID + " -> " + newNameWPath);
	return true;
}

std::string PrefabManager::getPrefabFilename(const std::string& prefabID) const {
	auto it = m_prefabRegistry.find(prefabID);
	if (it == m_prefabRegistry.end()) {
		DebugLog::addMessage("[Error] Prefab ID '" + prefabID + "' not found.");
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
		DebugLog::addMessage("PrefabManager::instantiate() failed: prefab ID not found: " + id);
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
			DebugLog::addMessage("Failed to load prefab file '" + path + "': " + err);
			return nullptr;
		}

		m_prefabCache[id].CopyFrom(doc, m_prefabCache[id].GetAllocator());
	}

	if (doc.HasMember("layer") && doc["layer"].IsInt()) {
		int layerID = doc["layer"].GetInt();
		manager.assignObjectToLayer(go, layerID);
	}

	if (doc.HasMember("components") && doc["components"].IsObject()) {
		applyPrefabComponents(go, doc["components"]);
	}

	/* --------- Load Component ---------*/
	//const auto& comps = doc["components"];

	// std::cout << "Instantiated prefab '" << "' (" << id << ")\n";
	return go;
}

std::unique_ptr<GameObject> PrefabManager::createTempPrefabObj(const std::string& prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		DebugLog::addMessage("Prefab not found in registry.");
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
			DebugLog::addMessage("Failed to load prefab file '" + path + "': " + err);
			return nullptr;
		}

		m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());
	}

	if (!doc.HasMember("components") || !doc["components"].IsObject()) {
		DebugLog::addMessage("Prefab file '" + path + "' missing COMPONENT object.");
		return nullptr;
	}

	// cache original prefab state before editing (for comparison after saving later)
	m_prefabBeforeEdit[prefabID].CopyFrom(doc, m_prefabBeforeEdit[prefabID].GetAllocator());

	// create a new temp obj
	std::unique_ptr<GameObject> tempGO = GameObjectManager::createTempGameObject(iterator->second.name);
	tempGO->getObjectPrefabID() = prefabID;

	if (doc.HasMember("layer") && doc["layer"].IsInt()) {
		int layerID = doc["layer"].GetInt();
		// store the layer value, do not register!!!
		tempGO->setLayer(layerID); 
	}

	// apply prefab changes on the obj
	applyPrefabComponents(tempGO.get(), doc["components"]);

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
	//const rapidjson::Document* prefabDoc{ getPrefabJson(prefabID) };

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
	serializeComponents(prefabObj, comps, a);

	doc.AddMember("components", comps, a);

	// save to runtime
	std::string path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
	std::string err;
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		DebugLog::addMessage("Prefab save failed (runtime): " + err);
		return false;
	}
	else {
		std::cout << "Prefab saved (runtime JSON): " << path << "\n";
	}

	// save to source
	path = std::string(SOURCE_DIR_R) + getPrefabPath(prefabID);
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		DebugLog::addMessage("Prefab save failed (source): " + err);
		return false;
	}
	else {
		std::cout << "Prefab saved (source JSON): " << path << "\n";
	}

	// update cache
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	// update registry incase of name change
	savePrefabRegistry();

	return true;
}

GameObject* PrefabManager::revertToPrefab(GameObject* obj, GameObjectManager& manager) {
	if (!obj) return obj;

	const std::string& prefabID = obj->getObjectPrefabID();
	if (prefabID.empty()) {
		DebugLog::addMessage("Error: Object has no prefab.");
		return obj;
	}

	// store scene data first (preserve position)
	Transform* t = obj->getComponent<Transform>();
	float posX = t ? t->x : 0.f;
	float posY = t ? t->y : 0.f;
	float posZ = t ? t->z : 0.f;

	std::string name = obj->getObjectName();
	int layer = obj->getLayer();

	// remove existing components
	obj->removeComponent<Transform>();
	obj->removeComponent<Render>();
	obj->removeComponent<Input>();
	obj->removeComponent<Physics>();
	obj->removeComponent<CollisionInfo>();
	obj->removeComponent<Animation>();
	obj->removeComponent<StateMachine>();
	obj->removeComponent<AudioComponent>();
	obj->removeComponent<FontComponent>();

	// reapply prefan
	this->instantiate(obj, manager);

	// restore the position
	if (Transform* newT = obj->getComponent<Transform>()) {
		newT->x = posX;
		newT->y = posY;
		newT->z = posZ;
	}

	obj->getObjectName() = name;
	manager.assignObjectToLayer(obj, layer);

	// --- Reinitialize resources if needed ---
	manager.initializeSceneResources();

	return obj;
}

// apply prefab changes to all existing instances in current scene
int PrefabManager::applyToAllInstances(const std::string& prefabID, GameObjectManager& manager) {
	if (prefabID.empty()) return 0;

	auto oldI = m_prefabBeforeEdit.find(prefabID); // before edit
	auto newI = m_prefabCache.find(prefabID); // aft edit and save

	if (oldI == m_prefabBeforeEdit.end()) {
		DebugLog::addMessage("No cached 'before edit' state for prefab: " + prefabID + "\n");
		DebugLog::addMessage("Falling back to simple reload (all overrides preserved)\n");

		// Try to load from disk as fallback
		const std::string& path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
		rapidjson::Document diskDoc;
		std::string err;
		if (JsonIO::ReadFileToDocument(path, diskDoc, &err)) {
			m_prefabBeforeEdit[prefabID].CopyFrom(diskDoc, m_prefabBeforeEdit[prefabID].GetAllocator());
			oldI = m_prefabBeforeEdit.find(prefabID);
		}
	}

	// if no cached before editing state found for prefab, fall back to original obj state
	if (oldI == m_prefabBeforeEdit.end()) {
		// PLEASE DONT HAPPEN
		DebugLog::addMessage("No cached state found for prefab.");
		DebugLog::addMessage("Fall back to simple reload (all overrides preserved.");

		if (newI == m_prefabCache.end()) return 0;

		std::vector<GameObject*> gameObjects;
		manager.getAllGameObjects(gameObjects);

		int count{};

		const rapidjson::Document& newPrefabDoc = newI->second;
		if (!newPrefabDoc.HasMember("components")) return 0;

		for (GameObject* obj : gameObjects) {
			// only affect those obj ref to a prefab
			if (obj->getObjectPrefabID() != prefabID) continue;

			rapidjson::Document objDoc;
			objDoc.SetObject();
			auto& a = objDoc.GetAllocator();

			rapidjson::Value comps(rapidjson::kObjectType);
			serializeComponents(obj, comps, a); // save obj to json to store its data
			objDoc.AddMember("components", comps, a);

			/*objDoc.AddMember("name", rapidjson::Value(obj->getObjectName().c_str(), a), a);
			objDoc.AddMember("layer", obj->getLayer(), a);*/

			// remove all existing components
			obj->removeComponent<Transform>();
			obj->removeComponent<Render>();
			obj->removeComponent<Input>();
			obj->removeComponent<Physics>();
			obj->removeComponent<CollisionInfo>();
			obj->removeComponent<Animation>();
			obj->removeComponent<StateMachine>();
			obj->removeComponent<AudioComponent>();
			obj->removeComponent<FontComponent>();

			applyPrefabComponents(obj, newPrefabDoc["components"]);

			if (objDoc.HasMember("components")) {
				applyPrefabComponents(obj, objDoc["components"]);
			}

			// reapply scene data
			if (Render* r = obj->getComponent<Render>()) {
				if (r->hasTex && !r->texFile.empty()) {
					r->texChanged = true;
				}
			}

			++count;
		}

		manager.initializeSceneResources();
		return count;
	}
	
	// cached found yay
	if (newI == m_prefabCache.end()) return 0;

	const rapidjson::Document& oldPrefabDoc = oldI->second;
	const rapidjson::Document& newPrefabDoc = newI->second;

	if (!oldPrefabDoc.HasMember("components") || !oldPrefabDoc["components"].IsObject()) {
		DebugLog::addMessage("Old prefab missing components object\n");
		return 0;
	}
	if (!newPrefabDoc.HasMember("components") || !newPrefabDoc["components"].IsObject()) {
		DebugLog::addMessage("New prefab missing components object\n");
		return 0;
	}

	int oldPrefabLayer = (oldPrefabDoc.HasMember("layer") && oldPrefabDoc["layer"].IsInt()) ? oldPrefabDoc["layer"].GetInt() : 1; // 1 is default game layer

	int newPrefabLayer = (newPrefabDoc.HasMember("layer") && newPrefabDoc["layer"].IsInt()) ? newPrefabDoc["layer"].GetInt() : 1;

	const rapidjson::Value& oldPrefabComps = oldPrefabDoc["components"];
	const rapidjson::Value& newPrefabComps = newPrefabDoc["components"];

	/*std::cout << "\nApplying Prefab Changes\n";
	std::cout << "Old prefab has " << oldPrefabComps.MemberCount() << " components\n";
	std::cout << "New prefab has " << newPrefabComps.MemberCount() << " components\n";*/

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	int count{};

	for (GameObject* obj : gameObjects) {
		if (obj->getObjectPrefabID() != prefabID) continue;

		std::string objName = obj->getObjectName();
		int objLayer = obj->getLayer();

		applyPrefabComponentsSelective(obj, oldPrefabComps, newPrefabComps);

		obj->getObjectName() = objName;
		manager.assignObjectToLayer(obj, objLayer);

		if (objLayer == oldPrefabLayer) {
			// Layer wasn't overridden, apply new prefab layer
			std::cout << "  Layer matches old prefab - applying new layer: " << newPrefabLayer << "\n";
			manager.assignObjectToLayer(obj, newPrefabLayer);
		}
		else {
			// Layer was overridden, keep instance's layer
			std::cout << "  Layer was overridden - keeping instance layer\n";
			manager.assignObjectToLayer(obj, objLayer);
		}

		if (Render* r = obj->getComponent<Render>()) {
			if (r->hasTex && !r->texFile.empty()) {
				r->texChanged = true;
			}
		}

		++count;
	}

	manager.initializeSceneResources();

	m_prefabBeforeEdit.erase(prefabID);

	return count;
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

	m_prefabRegistry[prefabID] = { prefabName, filename };

	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// prefab id
	doc.AddMember("id", rapidjson::Value(prefabID.c_str(), a), a);
	// name
	doc.AddMember("name", rapidjson::Value(prefabName.c_str(), a), a);

	rapidjson::Value comps(rapidjson::kObjectType);
	serializeComponents(go, comps, a);

	doc.AddMember("components", comps, a);

	/* -------- Save new prefab to a new .json file ------- */
	std::string path = std::string(RUNTIME_DIR_R) + getPrefabPath(prefabID);
	std::string err;
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		DebugLog::addMessage("Prefab save failed (runtime): " + err);
		return false;
	}
	else {
		std::cout << "Prefab saved (runtime JSON): " << path << "\n";
	}

	path = std::string(SOURCE_DIR_R) + getPrefabPath(prefabID);
	if (!JsonIO::WriteDocumentToFile(path, doc, /*pretty=*/true, &err)) {
		DebugLog::addMessage("Prefab save failed (source): " + err);
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
	m_prefabCache[prefabID].CopyFrom(doc, m_prefabCache[prefabID].GetAllocator());

	return true;
}

bool PrefabManager::deletePrefab(const std::string prefabID) {
	auto iterator = m_prefabRegistry.find(prefabID);
	if (iterator == m_prefabRegistry.end()) {
		DebugLog::addMessage("PrefabManager::deleteprefab: failed � prefab ID not found: " + prefabID);
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

void PrefabManager::applyPrefabComponents(GameObject* go, const rapidjson::Value& comps) {
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
		go->addComponent<Input>();
	}

	// ---- Physics ----
	if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
		const auto& jp = comps["Physics"];
		auto* p = go->addComponent<Physics>();

		if (jp.HasMember("physicsFlag")) {
			p->physicsFlag = jp["physicsFlag"].IsBool()
				? jp["physicsFlag"].GetBool()
				: (jp["physicsFlag"].GetInt() != 0);
		}

		if (jp.HasMember("moveSpeed") && jp["moveSpeed"].IsNumber())
			p->moveSpeed = jp["moveSpeed"].GetFloat();

		if (jp.HasMember("damping") && jp["damping"].IsNumber())
			p->damping = jp["damping"].GetFloat();

		if (jp.HasMember("jumpForce") && jp["jumpForce"].IsNumber())
			p->jumpForce = jp["jumpForce"].GetFloat();

		if (jp.HasMember("mass") && jp["mass"].IsNumber())
			p->dynamics.mass = jp["mass"].GetFloat();

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
		const auto& jc = comps["CollisionInfo"];
		auto* c = go->addComponent<CollisionInfo>();

		if (jc.HasMember("collisionFlag")) {
			c->collisionFlag = jc["collisionFlag"].IsBool()
				? jc["collisionFlag"].GetBool()
				: (jc["collisionFlag"].GetInt() != 0);
		}

		if (jc.HasMember("autoFitScale")) {
			c->autoFitScale = jc["autoFitScale"].IsBool()
				? jc["autoFitScale"].GetBool()
				: (jc["autoFitScale"].GetInt() != 0);
		}

		std::string shp = (jc.HasMember("colliderType") && jc["colliderType"].IsString())
			? jc["colliderType"].GetString()
			: "square";
		c->colliderType = JsonIO::StrToShape(shp);

		if (jc.HasMember("colliderSize") && jc["colliderSize"].IsArray() && jc["colliderSize"].Size() == 2)
		{
			c->colliderSize.x = jc["colliderSize"][0].GetFloat();
			c->colliderSize.y = jc["colliderSize"][1].GetFloat();
		}
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
	}

	// ---- AudioComponent ----
	if (comps.HasMember("AudioComponent") && comps["AudioComponent"].IsObject()) {
		const auto& ja = comps["AudioComponent"];
		auto* ac = go->addComponent<AudioComponent>();

		if (ja.HasMember("audioFile") && ja["audioFile"].IsString())
			ac->audioFile = ja["audioFile"].GetString();
		if (ja.HasMember("volume") && ja["volume"].IsNumber())
			ac->volume = ja["volume"].GetFloat();
		if (ja.HasMember("pitch") && ja["pitch"].IsNumber())
			ac->pitch = ja["pitch"].GetFloat();
		if (ja.HasMember("loop")) {
			ac->loop = ja["loop"].IsBool() ? ja["loop"].GetBool() : (ja["loop"].GetInt() != 0);
		}
		if (ja.HasMember("playOnStart")) {
			ac->playOnStart = ja["playOnStart"].IsBool() ? ja["playOnStart"].GetBool() : (ja["playOnStart"].GetInt() != 0);
		}
		if (ja.HasMember("muted")) {
			ac->muted = ja["muted"].IsBool() ? ja["muted"].GetBool() : (ja["muted"].GetInt() != 0);
		}
		if (ja.HasMember("fadeInOnStart")) {
			ac->fadeInOnStart = ja["fadeInOnStart"].IsBool() ? ja["fadeInOnStart"].GetBool() : (ja["fadeInOnStart"].GetInt() != 0);
		}
		if (ja.HasMember("fadeInDuration") && ja["fadeInDuration"].IsNumber())
			ac->fadeInDuration = ja["fadeInDuration"].GetFloat();
		if (ja.HasMember("fadeOutOnStop")) {
			ac->fadeOutOnStop = ja["fadeOutOnStop"].IsBool() ? ja["fadeOutOnStop"].GetBool() : (ja["fadeOutOnStop"].GetInt() != 0);
		}
		if (ja.HasMember("fadeOutDuration") && ja["fadeOutDuration"].IsNumber())
			ac->fadeOutDuration = ja["fadeOutDuration"].GetFloat();
	}

	// ---- FontComponent ----
	if (comps.HasMember("FontComponent") && comps["FontComponent"].IsObject()) {
		const auto& jf = comps["FontComponent"];
		auto* fc = go->addComponent<FontComponent>();

		if (jf.HasMember("word") && jf["word"].IsString())
			fc->word = jf["word"].GetString();
		if (jf.HasMember("scale") && jf["scale"].IsNumber())
			fc->scale = jf["scale"].GetFloat();
		if (jf.HasMember("fontType") && jf["fontType"].IsInt())
			fc->fontType = jf["fontType"].GetInt();
		if (jf.HasMember("color") && jf["color"].IsArray() && jf["color"].Size() == 3) {
			fc->clr.r = jf["color"][0].GetFloat();
			fc->clr.g = jf["color"][1].GetFloat();
			fc->clr.b = jf["color"][2].GetFloat();
		}
	}
}

void PrefabManager::serializeComponents(const GameObject* obj, rapidjson::Value& comps, rapidjson::Document::AllocatorType& a) {
	//// ---- Transform ----
	if (auto t = obj->getComponent<Transform>()) {
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
	if (auto r = obj->getComponent<Render>()) {
		rapidjson::Value jr(rapidjson::kObjectType);

		jr.AddMember("shape", rapidjson::Value(ShapeToStr(r->modelRef.shape), a), a);
		jr.AddMember("hasTex", r->hasTex ? 1 : 0, a);
		if (r->hasTex && !r->texFile.empty())
			jr.AddMember("texture", rapidjson::Value(r->texFile.c_str(), a), a);
		jr.AddMember("hasAnimation", r->hasAnimation ? 1 : 0, a);
		if (r->clr.r != 1.f || r->clr.g != 1.f || r->clr.b != 1.f) {
			rapidjson::Value clr(rapidjson::kArrayType);
			clr.PushBack(r->clr.r, a).PushBack(r->clr.g, a).PushBack(r->clr.b, a);
			jr.AddMember("clr", clr, a);
		}

		comps.AddMember("Render", jr, a);
	}

	if (auto i = obj->getComponent<Input>()) {
		comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
	}

	//// ---- Physics ----
	if (auto p = obj->getComponent<Physics>()) {
		rapidjson::Value jp(rapidjson::kObjectType);

		jp.AddMember("physicsFlag", p->physicsFlag ? 1 : 0, a);

		jp.AddMember("moveSpeed", p->moveSpeed, a);
		jp.AddMember("jumpForce", p->jumpForce, a);
		jp.AddMember("damping", p->damping, a);
		jp.AddMember("mass", p->dynamics.mass, a);

		jp.AddMember("inWater", p->inWater ? 1 : 0, a);
		jp.AddMember("buoyancy", p->buoancy ? 1 : 0, a);

		comps.AddMember("Physics", jp, a);
	}

	//// ---- Collision ----
	if (auto c = obj->getComponent<CollisionInfo>()) {
		rapidjson::Value jc(rapidjson::kObjectType);
		jc.AddMember("collisionFlag", c->collisionFlag ? 1 : 0, a);
		jc.AddMember("autoFitScale", c->autoFitScale ? 1 : 0, a);

		rapidjson::Value colliderArray(rapidjson::kArrayType);
		colliderArray.PushBack(c->colliderSize.x, a);colliderArray.PushBack(c->colliderSize.y, a);
		jc.AddMember("colliderSize", colliderArray, a);

		std::string shapeStr = JsonIO::ShapeToStr(c->colliderType);
		jc.AddMember("colliderType", rapidjson::Value(shapeStr.c_str(), a), a);

		comps.AddMember("CollisionInfo", jc, a);
	}

	//// ---- Animation ----
	if (auto an = obj->getComponent<Animation>()) {
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
	if (auto sm = obj->getComponent<StateMachine>()) {
		rapidjson::Value js(rapidjson::kObjectType);

		js.AddMember("state", rapidjson::Value(playerStateToStr(sm->state), a), a);
		js.AddMember("facingRight", sm->facingRight ? 1 : 0, a);
		js.AddMember("stateTime", sm->stateTime, a);

		comps.AddMember("StateMachine", js, a);
	}

	// AudioComponent
	if (auto ac = obj->getComponent<AudioComponent>()) {
		rapidjson::Value ja(rapidjson::kObjectType);

		ja.AddMember("audioFile", rapidjson::Value(ac->audioFile.c_str(), a), a);
		ja.AddMember("volume", ac->volume, a);
		ja.AddMember("pitch", ac->pitch, a);
		ja.AddMember("loop", ac->loop ? 1 : 0, a);
		ja.AddMember("playOnStart", ac->playOnStart ? 1 : 0, a);
		ja.AddMember("muted", ac->muted ? 1 : 0, a);
		ja.AddMember("fadeInOnStart", ac->fadeInOnStart ? 1 : 0, a);
		ja.AddMember("fadeInDuration", ac->fadeInDuration, a);
		ja.AddMember("fadeOutOnStop", ac->fadeOutOnStop ? 1 : 0, a);
		ja.AddMember("fadeOutDuration", ac->fadeOutDuration, a);

		comps.AddMember("AudioComponent", ja, a);
	}

	// FontComponent
	if (auto fc = obj->getComponent<FontComponent>()) {
		rapidjson::Value jf(rapidjson::kObjectType);
		jf.AddMember("word", rapidjson::Value(fc->word.c_str(), a), a);
		jf.AddMember("scale", fc->scale, a);
		jf.AddMember("fontType", fc->fontType, a);
		rapidjson::Value clr(rapidjson::kArrayType);
		clr.PushBack(fc->clr.r, a).PushBack(fc->clr.g, a).PushBack(fc->clr.b, a);
		jf.AddMember("color", clr, a);
		comps.AddMember("FontComponent", jf, a);
	}
}

bool PrefabManager::valuesEqual(const rapidjson::Value& lhs, const rapidjson::Value& rhs) const {
	// make sure lhs and rhs are the same type before continue
	if (lhs.GetType() != rhs.GetType()) return false;

	if (lhs.IsNull()) return rhs.IsNull();

	if (lhs.IsBool()) return lhs.GetBool() == rhs.GetBool();

	if (lhs.IsInt()) return lhs.GetInt() == rhs.GetInt();

	if (lhs.IsDouble()) {
		return std::abs(lhs.GetDouble() - rhs.GetDouble()) < 0.0001;
	}

	if (lhs.IsString()) {
		return std::strcmp(lhs.GetString(), rhs.GetString()) == 0;
	}

	if (lhs.IsArray()) {
		if (lhs.Size() != rhs.Size()) return false;
		for (rapidjson::SizeType i = 0; i < lhs.Size(); ++i) {
			if (!valuesEqual(lhs[i], rhs[i])) return false;
		}
		return true;
	}

	if (lhs.IsObject()) {
		if (lhs.MemberCount() != rhs.MemberCount()) return false;
		for (auto it = lhs.MemberBegin(); it != lhs.MemberEnd(); ++it) {
			if (!rhs.HasMember(it->name.GetString())) return false;
			if (!valuesEqual(it->value, rhs[it->name.GetString()])) return false;
		}
		return true;
	}

	return false;
}

void PrefabManager::applyPrefabComponentsSelective(GameObject* obj, const rapidjson::Value& oldPrefabComps, const rapidjson::Value& newPrefabComps) {
	rapidjson::Document objDoc;
	objDoc.SetObject();
	auto& a = objDoc.GetAllocator();

	rapidjson::Value objComps(rapidjson::kObjectType);
	serializeComponents(obj, objComps, a);

	// track which components exist in new prefab
	std::unordered_set<std::string> newPrefabCompSet;
	for (auto it = newPrefabComps.MemberBegin(); it != newPrefabComps.MemberEnd(); ++it) {
		newPrefabCompSet.insert(it->name.GetString());
	}

	// remove component that no longer in new prefab (means removed)
	std::vector<std::string> componentsToRemove;
	for (auto it = objComps.MemberBegin(); it != objComps.MemberEnd(); ++it) {
		const char* compName = it->name.GetString();

		// if this component existed in old prefab but NOT in new prefab, remove it
		if (oldPrefabComps.HasMember(compName) && !newPrefabCompSet.count(compName)) {
			componentsToRemove.push_back(compName);
		}
	}

	// remove the components
	for (const std::string& compName : componentsToRemove) {
		if (compName == "Transform") obj->removeComponent<Transform>();
		else if (compName == "Render") obj->removeComponent<Render>();
		else if (compName == "Input") obj->removeComponent<Input>();
		else if (compName == "Physics") obj->removeComponent<Physics>();
		else if (compName == "CollisionInfo") obj->removeComponent<CollisionInfo>();
		else if (compName == "Animation") obj->removeComponent<Animation>();
		else if (compName == "StateMachine") obj->removeComponent<StateMachine>();
		else if (compName == "AudioComponent") obj->removeComponent<AudioComponent>();
		else if (compName == "FontComponent") obj->removeComponent<FontComponent>();
	}

	// for each component in new prefab
	for (auto compI = newPrefabComps.MemberBegin(); compI != newPrefabComps.MemberEnd(); ++compI) {
		const char* compName = compI->name.GetString();
		const rapidjson::Value& newCompData = compI->value;

		// if new prefab has a new component
		if (!oldPrefabComps.HasMember(compName)) {
			//std::cout << "  Component '" << compName << "' is NEW in prefab!! applying fully\n\n";

			// remove if exists (to start fresh)
			if (compName == std::string("Transform")) obj->removeComponent<Transform>();
			else if (compName == std::string("Render")) obj->removeComponent<Render>();
			else if (compName == std::string("Input")) obj->removeComponent<Input>();
			else if (compName == std::string("Physics")) obj->removeComponent<Physics>();
			else if (compName == std::string("CollisionInfo")) obj->removeComponent<CollisionInfo>();
			else if (compName == std::string("Animation")) obj->removeComponent<Animation>();
			else if (compName == std::string("StateMachine")) obj->removeComponent<StateMachine>();
			else if (compName == std::string("AudioComponent")) obj->removeComponent<AudioComponent>();
			else if (compName == std::string("FontComponent")) obj->removeComponent<FontComponent>();

			// apply new component
			rapidjson::Value temp(rapidjson::kObjectType);
			rapidjson::Value copy;
			copy.CopyFrom(newCompData, a);
			temp.AddMember(rapidjson::Value(compName, a), copy, a);

			applyPrefabComponents(obj, temp);

			continue;
		}


		const rapidjson::Value& oldCompData = oldPrefabComps[compName];


		// if obj doesnt have this component
		if (!objComps.HasMember(compName)) {
			/*std::cout << "  Object missing component '" << compName << "', applying from prefab\n";*/

			rapidjson::Value temp(rapidjson::kObjectType);
			rapidjson::Value copy;
			copy.CopyFrom(newCompData, a);
			temp.AddMember(rapidjson::Value(compName, a), copy, a);

			applyPrefabComponents(obj, temp);

			continue;
		}


		const rapidjson::Value& objCompData = objComps[compName];


		// component exists in all three -> compare properties
		rapidjson::Value finalCompData(rapidjson::kObjectType);
		for (auto propIt = newCompData.MemberBegin(); propIt != newCompData.MemberEnd(); ++propIt) {
			const char* propName = propIt->name.GetString();
			const rapidjson::Value& newValue = propIt->value;

			// property is NEW in the prefab (shouldnt be??)
			if (!oldCompData.HasMember(propName)) {
				/*std::cout << "    Property '" << compName << "." << propName << "' is NEW - using prefab value\n";*/

				rapidjson::Value copy;
				copy.CopyFrom(newValue, a);

				finalCompData.AddMember(rapidjson::Value(propName, a), copy, a);

				continue;
			}

			const rapidjson::Value& oldValue = oldCompData[propName];

			// object doesn't have this property (shouldnt be??)
			if (!objCompData.HasMember(propName)) {
				/*std::cout << "    Property '" << compName << "." << propName << "' missing in object - using prefab value\n";*/

				rapidjson::Value copy;
				copy.CopyFrom(newValue, a);

				finalCompData.AddMember(rapidjson::Value(propName, a), copy, a);

				continue;
			}

			const rapidjson::Value& objValue = objCompData[propName];

			// compare values 
			if (valuesEqual(objValue, oldValue)) {
				// object value matches old prefab -> not override -> use new prefab value
				/*std::cout << "    Property '" << compName << "." << propName << "' matches old prefab - applying NEW value\n";*/

				rapidjson::Value copy;
				copy.CopyFrom(newValue, a);

				finalCompData.AddMember(rapidjson::Value(propName, a), copy, a);
			}
			else {
				// object value differs from old prefab -> override -> keep object value
				/*std::cout << "    Property '" << compName << "." << propName << "' was OVERRIDDEN - keeping object value\n";*/

				rapidjson::Value copy;
				copy.CopyFrom(objValue, a);

				finalCompData.AddMember(rapidjson::Value(propName, a), copy, a);
			}
		}

		// remove and add back component with final data
		if (compName == std::string("Transform")) obj->removeComponent<Transform>();
		else if (compName == std::string("Render")) obj->removeComponent<Render>();
		else if (compName == std::string("Input")) obj->removeComponent<Input>();
		else if (compName == std::string("Physics")) obj->removeComponent<Physics>();
		else if (compName == std::string("CollisionInfo")) obj->removeComponent<CollisionInfo>();
		else if (compName == std::string("Animation")) obj->removeComponent<Animation>();
		else if (compName == std::string("StateMachine")) obj->removeComponent<StateMachine>();
		else if (compName == std::string("AudioComponent")) obj->removeComponent<AudioComponent>();
		else if (compName == std::string("FontComponent")) obj->removeComponent<FontComponent>();

		rapidjson::Value temp(rapidjson::kObjectType);
		temp.AddMember(rapidjson::Value(compName, a), finalCompData, a);

		applyPrefabComponents(obj, temp);
	}


	// preserve scene-specific components
	//for (auto it = objComps.MemberBegin(); it != objComps.MemberEnd(); ++it) {
	//	const char* compName = it->name.GetString();

	//	// scene-specific component
	//	if (!oldPrefabComps.HasMember(compName) && !newPrefabCompSet.count(compName)) {
	//		//std::cout << "  Component '" << compName << "' is SCENE-ONLY - preserving\n";
	//		// component already exists, no action needed
	//	}
	//}
}