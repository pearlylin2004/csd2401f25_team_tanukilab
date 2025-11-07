/* Start Header ************************************************************************/
/*!
\file        GameObjectManager.cpp
\author      Seow Sin Le, s.sinle, 2401084, 50%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 50%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\date        October, 1st, 2025
\brief       Manages all game objects and initializes them with their corresponding components

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameObjectManager.h"
#include "config.h"
#include "prefabManager.h"
#include "Component.h"
#include "JsonIO.h"

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./assets/Scene"
#endif
#ifndef SOURCE_ASSETS_DIR_R
#define SOURCE_ASSETS_DIR_R "./projects/WaterBound/assets"
#endif

namespace {
	inline bool ends_with(const std::string& s, const char* suf) {
		const size_t n = std::strlen(suf);
		return s.size() >= n && s.compare(s.size() - n, n, suf) == 0;
	}
}

// Create objects
GameObject* GameObjectManager::createGameObject(const std::string& name) {

	// Create a new GameObject and wrap it in a unique_ptr
	auto newObject = std::make_unique<GameObject>(name);


	// Get raw pointer before transferring ownership
	GameObject* ptr = newObject.get();


	//assign to default layer (game layer)
	m_layerManager.assignObjectToLayer(ptr, 1);

	// Store the unique_ptr in the map
	m_gameObjects[name] = std::move(newObject);


	return ptr;
}

std::unique_ptr<GameObject> GameObjectManager::createTempGameObject(const std::string& name) {
	return std::make_unique<GameObject>(name);
}

//use this when objects need to interact with each other
GameObject* GameObjectManager::getGameObject(const std::string& name) {

	//find object in map
	auto iterator = m_gameObjects.find(name);
	if (iterator != m_gameObjects.end()) {//if found
		return iterator->second.get();
	}

	return nullptr;
}

void GameObjectManager::deleteGameObject(GameObject* object) {
	if (!object) return;

	m_layerManager.removeObjectFromLayer(object);

	for (auto iterator = m_gameObjects.begin(); iterator != m_gameObjects.end(); ++iterator) {
		if (iterator->second.get() == object) {
			m_gameObjects.erase(iterator);
			return;
		}
	}
}

//clone an existing object
GameObject* GameObjectManager::CloneGameObject(const std::string& sourceName, const std::string& newName) {
	//find object in map
	auto iterator = m_gameObjects.find(sourceName);

	if (iterator == m_gameObjects.end()) {
		std::cout << "GameObject with name '" << sourceName << "' not found.\n";
		return nullptr;
	}

	if (m_gameObjects.count(newName)) {
		std::cout << "GameObject with name '" << newName << "' already exists.\n";
		return nullptr;
	}
	
	//cloning GameObject
	std::unique_ptr<GameObject> clonedObject = iterator->second->clone(newName);

	GameObject* ptr = clonedObject.get();//raw pointer

	m_gameObjects[newName] = std::move(clonedObject);//move into map

	return ptr;//return the raw pointer
}

//used for the all the component systems
void GameObjectManager::getAllGameObjects(std::vector<GameObject*>& gameObjects) {
	//clear vector just in case
	gameObjects.clear();

	gameObjects.reserve(m_gameObjects.size());

	for (const auto& iterator : m_gameObjects) {
		gameObjects.push_back(iterator.second.get());
	}
}

void GameObjectManager::init()
{

	loadScene(JsonIO::runtimeScenePath("level01.json"));
}

bool GameObjectManager::renameGameObject(GameObject* obj, const std::string& newName) {
	if (!obj) return false;
	if (newName.empty()) return false;

	for (auto& odject : m_gameObjects) {
		// return false another object has the same name
		GameObject* other = odject.second.get();
		if (other != obj && other->getObjectName() == newName) {
			return false;
		}
	}

	// rename if no duplicated name
	obj->getObjectName() = newName;
	return true;

}

void GameObjectManager::loadFromFile(const std::string& filename)
{
	std::ifstream is{ filename };
	if (!is)
	{
		std::cerr << "file not found: " + filename + "\n";
		return;
	}
	std::cout << "file opened\n";

	m_layerManager.clearAllLayers();
	m_gameObjects.clear(); // clear existing objects before loading new ones

	std::string checker{};
	std::string objectName;
	std::string shape;
	while (is >> checker)
	{
		/*for (; is >> checker, checker == "object";)
			if (checker == "object") break;*/
		if (checker == "object")
		{
			is >> objectName;
			std::unique_ptr<GameObject> newObject = std::make_unique<GameObject>(objectName);
			m_gameObjects.emplace(objectName, std::move(newObject));
			std::cout << "\nnew object:\n";
			std::cout << objectName << "\n";
		}
		/*for (; is >> checker, checker == "renderer";)
			if (checker == "renderer") break;*/
		if (checker == "renderer")
		{
			Render* renderptr = m_gameObjects[objectName]->addComponent<Render>();
			is >> shape;
			if (shape == "square")
			{
				renderptr->modelRef = renderer::models[static_cast<int>(shape::square)];
			}
			if (shape == "circle")
			{
				renderptr->modelRef = renderer::models[static_cast<int>(shape::circle)];
			}
			is >> renderptr->hasTex;
			if (renderptr->hasTex == true)
			{
				is >> renderptr->texFile;
				renderptr->texHDL = 0;
				renderptr->texChanged = false;
				is >> renderptr->hasAnimation;
			}
			else {
				renderptr->texFile.clear();
				renderptr->texHDL = 0;
				renderptr->texChanged = false;
			}
			std::cout << shape << "\n";
		}
		/*for (; is >> checker, checker == "transform";)
			if (checker == "transform") break;*/
		if (checker == "transform")
		{
			Transform* transformptr = m_gameObjects[objectName]->addComponent<Transform>();
			is >> transformptr->x >> transformptr->y >> transformptr->z;
			is >> transformptr->rotation;
			is >> transformptr->scaleX >> transformptr->scaleY >> transformptr->scaleZ;
			std::cout << "transform :" << transformptr->x << " ," << transformptr->y << " ," << transformptr->z << "\n";
			std::cout << "rotation :" << transformptr->rotation << "\n";
			std::cout << "scale :" << transformptr->scaleX << " ," << transformptr->scaleY << " ," << transformptr->scaleZ << "\n";
		}
		if (checker == "input")
		{
			m_gameObjects[objectName]->addComponent<Input>(1.f);
			std::cout << "input component added\n";
		}
		if (checker == "physics") {
			Physics* phyptr = m_gameObjects[objectName]->addComponent<Physics>();
			is >> phyptr->inWater >> phyptr->buoancy;
			std::cout << "inwater : " << phyptr->inWater << ", " << phyptr->buoancy << "\n";
		}
		if (checker == "collision")
		{
			m_gameObjects[objectName]->addComponent<CollisionInfo>();
			std::cout << "collision component added\n";
		}

		if (checker == "animation")
		{
			Animation* animationptr = m_gameObjects[objectName]->addComponent<Animation>();
			is >> animationptr->loop >> animationptr->currentFrameColumn >> animationptr->currentFrameRow >> animationptr->totalColumn >> animationptr->totalRow >> animationptr->frameTime;
			std::cout << "animation component added\n";
		}

		if (checker == "font")
		{
			FontComponent* fc = m_gameObjects[objectName]->addComponent<FontComponent>();
			std::string line;
			std::getline(is >> std::ws, line);
			std::istringstream iss(line);
			iss >> std::quoted(fc->word) >> fc->clr.r >> fc->clr.g >> fc->clr.b >> fc->scale;
		}

		//m_gameObjects["player"]->addComponent<Input>(1.f);
		
	}
}

void GameObjectManager::saveToFile(const std::string& filename) const {
	std::ofstream os{ filename };
	if (!os)
	{
		std::cerr << "file not found: " << filename << std::endl;
		return;
	}

	for (auto& [name, obj] : m_gameObjects)
	{
		os << "object " << obj->getObjectName() << "\n";

		if (auto render = obj->getComponent<Render>())
		{
			os << "renderer ";

			switch (render->modelRef.shape) {
			case shape::square:
				os << "square ";
				break;
			case shape::circle:
				os << "circle ";
				break;
			default:
				os << "circle ";
				break;
			}

			os << render->hasTex << " ";
			if (render->hasTex)
				os << render->texFile;

			if(render->hasAnimation)
				os << " " << render->hasAnimation;
			else
				os << " " << 0;

			os << "\n";
		}

		if (auto transform = obj->getComponent<Transform>())
		{
			os << "transform "
				<< transform->x << " "
				<< transform->y << " "
				<< transform->z << " "
				<< transform->rotation << " "
				<< transform->scaleX << " "
				<< transform->scaleY << " "
				<< transform->scaleZ << "\n";
		}

		if (obj->hasComponent<Input>())
		{
			os << "input\n";
		}

		if (auto physics = obj->getComponent<Physics>())
		{
			os << "physics "
				<< physics->inWater << " "
				<< physics->buoancy << "\n";
		}

		if (obj->hasComponent<CollisionInfo>())
		{
			os << "collision\n";
		}

		if (Animation* animationptr = obj->getComponent<Animation>()) {
			os << "animation "
				<< animationptr->loop << " "
				<< animationptr->currentFrameColumn << " "
				<< animationptr->currentFrameRow << " "
				<< animationptr->totalColumn << " "
				<< animationptr->totalRow << " "
				<< animationptr->frameTime << "\n";
		}

		os << "\n";
	}

	std::cout << "Scene saved to " << filename << "\n";
}

/*!
\brief Converts a string into a PlayerState enum value.

This helper is used when loading state machine data from JSON.
Recognized values: "Idle", "Walking", "Jumping", "Falling", "Shooting".

\param s Input string from JSON.
\return Corresponding PlayerState value. Defaults to Idle if unknown.
*/

static PlayerState parsePlayerState(const std::string& s) {
	if (s == "Idle")     return PlayerState::Idle;
	if (s == "Walking")  return PlayerState::Walking;
	if (s == "Jumping")  return PlayerState::Jumping;
	if (s == "Falling")  return PlayerState::Falling;
	if (s == "Shooting") return PlayerState::Shooting;
	return PlayerState::Idle; // default/fallback
}
/*!
\brief Converts a PlayerState enum value into a string.

This function is used when saving state machine data back to JSON.

\param state The PlayerState to convert.
\return A string representation of the state (Idle, Walking, Jumping, etc.).
*/

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



/*!
\brief Loads a scene from a JSON file and rebuilds all game objects.

This function:
- Reads and parses JSON file content.
- Destroys old objects and clears old textures.
- Instantiates new objects, including prefab-based objects.
- Applies layer assignments.
- Overrides prefab defaults with scene-specific component data.
- Loads components such as Transform, Render, Physics, Animation,
  CollisionInfo, Input, StateMachine, and FontComponent.

If the JSON is missing or malformed, the function prints an error and stops.

\param filename Path to the JSON scene file.
*/

void GameObjectManager::loadSceneFromJson(const std::string& filename) {

	rapidjson::Document doc;
	std::string err;
	if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
		std::cerr << "Scene load failed: " << err << "\n";
		return;
	}

	int version = 1;
	if (doc.HasMember("version") && doc["version"].IsInt())
		version = doc["version"].GetInt();
	if (version != 1)
		std::cerr << "Warning: unsupported scene version " << version << "\n";

	/*for (auto& pair : m_gameObjects) {
		GameObject* obj = pair.second.get();
		if (obj && obj->hasComponent<Render>()) {
			Render* render = obj->getComponent<Render>();
			if (render->texHDL && render->hasTex) {
				glDeleteTextures(1, &render->texHDL);
				render->texHDL = 0;
				render->texChanged = false;
			}
		}
	}*/

	//std::cerr << "Cleared textures before loading scene.\n";
	
	m_layerManager.clearAllLayers();
	m_gameObjects.clear();

	if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
		std::cerr << "Scene JSON missing 'objects' array.\n";
		return;
	}

	for (auto& jObj : doc["objects"].GetArray()) {
		if (!jObj.IsObject()) continue;

		std::string nameOjb = (jObj.HasMember("name") && jObj["name"].IsString())
			? jObj["name"].GetString()
			: ("Object_" + std::to_string(m_gameObjects.size()));

		GameObject* go = createGameObject(nameOjb);


		// check if this object references a prefab
		if (jObj.HasMember("prefabid") && jObj["prefabid"].IsString()) {
			std::string prefabID = jObj["prefabid"].GetString();
			go->getObjectPrefabID() = prefabID;
			PrefabManager::Instance().instantiate(go, *this);

		}
		else {
			// if no prefab, just create a fresh game object
			std::string name = (jObj.HasMember("name") && jObj["name"].IsString())
				? jObj["name"].GetString()
				: ("Object_" + std::to_string(m_gameObjects.size()));
		}

		if (jObj.HasMember("layer") && jObj["layer"].IsInt()) {
			int layerID = jObj["layer"].GetInt();
			m_layerManager.assignObjectToLayer(go, layerID);
		}

		/*
		For obj referencing prefab -> this will override any scene-specific components, if nothing in scene data to override then will continue
		For obj without referencing prefab -> this simply set up its components
		*/
		if (!jObj.HasMember("components") || !jObj["components"].IsObject() || jObj["components"].ObjectEmpty())
			continue;

		const auto& comps = jObj["components"];

		// ---- Transform ----
		if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
			const auto& jt = comps["Transform"];

			auto* t = go->getComponent<Transform>();
			if (!t) t = go->addComponent<Transform>();

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

			/*std::cout << "\n\nReading from scene...\n";
			std::cout << "Transform: x: " << t->x << ", y: " << t->y << ", z: " << t->z << std::endl;
			std::cout << "Rotation: " << t->rotation << std::endl;
			std::cout << "Scale: x: " << t->scaleX << ", y: " << t->scaleY << ", z: " << t->scaleZ << std::endl;*/
		}

		// ---- Render ----
		if (comps.HasMember("Render") && comps["Render"].IsObject()) {
			const auto& jr = comps["Render"];
			
			auto* r = go->getComponent<Render>();
			if (!r) r = go->addComponent<Render>();

			// shape
			std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
				? jr["shape"].GetString()
				: "square";
			r->modelRef = renderer::models[static_cast<int>(JsonIO::StrToShape(shp))];

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


//          if (jr.HasMember("visible")     && jr["visible"].IsBool())     r->visible       = jr["visible"].GetBool();
//          if (jr.HasMember("transparent") && jr["transparent"].IsBool()) r->isTransparent = jr["transparent"].GetBool();
//          if (jr.HasMember("wireframe")   && jr["wireframe"].IsBool())   r->wireframe     = jr["wireframe"].GetBool();
		}

		// ---- Input (presence only) ----
		if (comps.HasMember("Input") && comps["Input"].IsObject()) {
			auto* i = go->getComponent<Input>();
			if (!i) i = go->addComponent<Input>(1.f);
		}

		// ---- Physics ----
		if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
			const auto& jp = comps["Physics"];
			
			auto* p = go->getComponent<Physics>();
			if (!p) p = go->addComponent<Physics>();

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
			auto* c = go->getComponent<CollisionInfo>();
			if (!c)
				c = go->addComponent<CollisionInfo>();
		}

		// ---- Animation ----
		if (comps.HasMember("Animation") && comps["Animation"].IsObject()) {
			const auto& ja = comps["Animation"];

			auto* a = go->getComponent<Animation>();
			if (!a)
				a = go->addComponent<Animation>();

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

			auto* sm = go->getComponent<StateMachine>();
			if (!sm)
				sm = go->addComponent<StateMachine>();

			if (smj.HasMember("state") && smj["state"].IsString())
				sm->state = parsePlayerState(smj["state"].GetString());

			if (smj.HasMember("facingRight")) {
				if (smj["facingRight"].IsBool())       sm->facingRight = smj["facingRight"].GetBool();
				else if (smj["facingRight"].IsInt())   sm->facingRight = (smj["facingRight"].GetInt() != 0);
			}

			if (smj.HasMember("stateTime") && smj["stateTime"].IsNumber())
				sm->stateTime = static_cast<float>(smj["stateTime"].GetDouble());

			/*std::cout << "\n\nReading scene...\n";
			std::cout << "State: " << sm->facingRight << std::endl;
			std::cout << "State Time: " << sm->stateTime << std::endl;*/
		}

		//font component
		if (comps.HasMember("FontComponent") && comps["FontComponent"].IsObject()) {
			const auto& jf = comps["FontComponent"];
			auto* fc = go->addComponent<FontComponent>();

			// word/text
			if (jf.HasMember("word") && jf["word"].IsString()) {
				fc->word = jf["word"].GetString();
			}

			// color (RGB array)
			if (jf.HasMember("color") && jf["color"].IsArray() && jf["color"].Size() == 3) {
				fc->clr.r = jf["color"][0].GetFloat();
				fc->clr.g = jf["color"][1].GetFloat();
				fc->clr.b = jf["color"][2].GetFloat();
			}

			// scale
			if (jf.HasMember("scale") && jf["scale"].IsNumber()) {
				fc->scale = jf["scale"].GetFloat();
			}

			// fontType (optional, default 1)
			if (jf.HasMember("fontType") && jf["fontType"].IsInt()) {
				fc->fontType = jf["fontType"].GetInt();
			}
			else {
				fc->fontType = 1;  // Default to ARIAL
			}

			
		}
		
	}

	std::cout << "Total game objects loaded: " << m_gameObjects.size() << std::endl;

	m_buttonData.clear(); // Ensure it's empty before populating

	// Extract from loaded game objects with "button" in the name
	//std::cout << "\nScanning loaded game objects for buttons..." << std::endl;
	int foundCount = 0;

	for (const auto& [name, obj] : m_gameObjects) {
		//std::cout << "  Checking object: " << name << std::endl;

		std::string lowerName = name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

		if (lowerName.find("button") != std::string::npos) {
			//std::cout << "    → Found button object!" << std::endl;
			foundCount++;

			if (auto* transform = obj->getComponent<Transform>()) {
				ButtonData data;
				data.position = glm::vec3(transform->x, transform->y, transform->z);
				data.scale = glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ);

				// Only add if not already added from buttons array
				if (m_buttonData.find(name) == m_buttonData.end()) {
					m_buttonData[name] = data;
					/*std::cout << "    ✓ Extracted from object: pos(" << data.position.x << ", "
						<< data.position.y << ", " << data.position.z << "), scale("
						<< data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;*/
				}
				else {
					std::cout << "Already loaded from buttons array" << std::endl;
				}
			}
			else {
				std::cout << " No Transform component!" << std::endl;
			}
		}
	}

	//std::cout << "\nButton objects found during scan: " << foundCount << std::endl;
	//std::cout << "Total button data entries: " << m_buttonData.size() << std::endl;

	if (m_buttonData.empty()) {
		std::cout << "WARNING: No button data was extracted!" << std::endl;
	}
	else {
		//std::cout << "SUCCESS: Button data loaded:" << std::endl;
		for (const auto& [name, data] : m_buttonData) {
			std::cout << "  - " << name << ": pos(" << data.position.x << ", "
				<< data.position.y << ", " << data.position.z << "), scale("
				<< data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
		}
	}

	//std::cout << "Scene loaded (JSON): " << filename << "\n";
}
/*!
\brief Saves all current game objects and component data to a JSON file.

This function:
- Creates a JSON document with version and object list.
- Includes prefab IDs when the object originates from a prefab.
- Saves only component values that differ from prefab defaults,
  minimizing redundant scene data.
- Exports components such as Transform, Render, Physics, Animation,
  CollisionInfo, Input, StateMachine, and FontComponent.

If writing fails, an error message is printed.

\param filename Output JSON file path.
\param isNew If true, writes an empty scene structure with only version header.
*/

void GameObjectManager::saveSceneToJson(const std::string& filename, bool isNew) const {
	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	PrefabManager& PM = PrefabManager::Instance();

	// version header
	doc.AddMember("version", rapidjson::Value(1), a);

	if (!isNew) {

		// objects array
		rapidjson::Value objects(rapidjson::kArrayType);

		for (auto const& kv : m_gameObjects) {
			const GameObject* obj = kv.second.get();

			rapidjson::Value jObj(rapidjson::kObjectType);

			// prefab ID
			const std::string& prefabID = obj->getObjectPrefabID();
			if (!prefabID.empty()) {
				jObj.AddMember("prefabid", rapidjson::Value(prefabID.c_str(), a), a);
			}

			// name
			jObj.AddMember("name", rapidjson::Value(obj->getObjectName().c_str(), a), a);


			int layerID = obj->getLayer();
			if (layerID != -1 && layerID != 1) {  // Only save if not default game layer
				jObj.AddMember("layer", layerID, a);
			}

			// get prefab JSON if obj referencing a prefab
			const rapidjson::Document* prefabDoc{ nullptr };
			if (!prefabID.empty()) {
				prefabDoc = PrefabManager::Instance().getPrefabJson(prefabID);
			}

			/* ------------ Useful lambda for comparing later ------------ */
			// obj referencing prefab -> if new data added for scene, will override prefab data
			// compare 2 float
			auto differsFloat = [](float a, float b) { return std::fabs(a - b) > 1e-6f; };
			// compare 2 int
			auto differsInt = [](int a, int b) { return a != b; };
			// compare 2 boolean
			auto differsBool = [](bool a, bool b) { return a != b; };

			// components container
			rapidjson::Value comps(rapidjson::kObjectType);

			// ---- Transform ----
			if (auto t = obj->getComponent<Transform>()) {
				rapidjson::Value jt(rapidjson::kObjectType);

				// 
				const rapidjson::Value* prefabT = PM.getPrefabJsonComp(prefabDoc, "Transform");

				if (!prefabT || differsFloat((*prefabT)["pos"][0].GetFloat(), t->x) ||
					differsFloat((*prefabT)["pos"][1].GetFloat(), t->y) ||
					differsFloat((*prefabT)["pos"][2].GetFloat(), t->z)) {
					rapidjson::Value pos(rapidjson::kArrayType);
					pos.PushBack(t->x, a).PushBack(t->y, a).PushBack(t->z, a);
					jt.AddMember("pos", pos, a);
				}

				if (!prefabT || !prefabT->HasMember("rotation") || differsFloat((*prefabT)["rotation"].GetFloat(), t->rotation))
					jt.AddMember("rotation", t->rotation, a);

				if (!prefabT || !prefabT->HasMember("scale") ||
					differsFloat((*prefabT)["scale"][0].GetFloat(), t->scaleX) ||
					differsFloat((*prefabT)["scale"][1].GetFloat(), t->scaleY) ||
					differsFloat((*prefabT)["scale"][2].GetFloat(), t->scaleZ)) {
					rapidjson::Value sc(rapidjson::kArrayType);
					sc.PushBack(t->scaleX, a).PushBack(t->scaleY, a).PushBack(t->scaleZ, a);
					jt.AddMember("scale", sc, a);
				}

				if (!jt.ObjectEmpty()) comps.AddMember("Transform", jt, a);
			}

			// ---- Render ----
			if (auto r = obj->getComponent<Render>()) {
				rapidjson::Value jr(rapidjson::kObjectType);
				const rapidjson::Value* prefabR = PM.getPrefabJsonComp(prefabDoc, "Render");

				std::string shapeStr = JsonIO::ShapeToStr(r->modelRef.shape);
				if (!prefabR || !prefabR->HasMember("shape") || shapeStr != (*prefabR)["shape"].GetString())
					jr.AddMember("shape", rapidjson::Value(shapeStr.c_str(), a), a);

				auto pushIfDiffBool = [&](const char* key, bool val) {
					if (!prefabR || !prefabR->HasMember(key) ||
						((*prefabR)[key].IsBool() && differsBool((*prefabR)[key].GetBool(), val)) ||
						((*prefabR)[key].IsInt() && differsBool((*prefabR)[key].GetInt() != 0, val))) {
						jr.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
					}
					};

				// save hasTex based on whether have texture
				pushIfDiffBool("hasTex", r->hasTex);
				if (r->hasTex && !r->texFile.empty()) {
					if (!prefabR || !prefabR->HasMember("texture") || r->texFile != (*prefabR)["texture"].GetString()) {
						jr.AddMember("texture", rapidjson::Value(r->texFile.c_str(), a), a);
						jr.AddMember("hasTex", rapidjson::Value(r->hasTex ? 1 : 0), a);
					}

				}

				pushIfDiffBool("hasAnimation", r->hasAnimation);

				if (!jr.ObjectEmpty()) comps.AddMember("Render", jr, a);
			}

			// ---- Physics ----
			if (auto p = obj->getComponent<Physics>()) {
				rapidjson::Value jp(rapidjson::kObjectType);
				const rapidjson::Value* prefabP = PM.getPrefabJsonComp(prefabDoc, "Physics");

				auto pushIfDiffBool = [&](const char* key, bool val) {
					if (!prefabP || !prefabP->HasMember(key) ||
						((*prefabP)[key].IsBool() && differsBool((*prefabP)[key].GetBool(), val)) ||
						((*prefabP)[key].IsInt() && differsBool((*prefabP)[key].GetInt() != 0, val))) {
						jp.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
					}
					};

				// if different then change scene data
				pushIfDiffBool("inWater", p->inWater);
				pushIfDiffBool("buoyancy", p->buoancy);

				if (!jp.ObjectEmpty()) comps.AddMember("Physics", jp, a);
			}

			// ---- Input (presence only) ----
			if (obj->hasComponent<Input>()) {
				const rapidjson::Value* prefabI = PM.getPrefabJsonComp(prefabDoc, "Input");
				if (!prefabI) comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
			}

			// ---- CollisionInfo (presence only) ----
			if (obj->hasComponent<CollisionInfo>()) {
				const rapidjson::Value* prefabC = PM.getPrefabJsonComp(prefabDoc, "CollisionInfo");
				if (!prefabC) comps.AddMember("CollisionInfo", rapidjson::Value(rapidjson::kObjectType), a);
			}

			// ---- Animation ----
			if (auto an = obj->getComponent<Animation>()) {
				rapidjson::Value ja(rapidjson::kObjectType);
				const rapidjson::Value* prefabA = PM.getPrefabJsonComp(prefabDoc, "Animation");

				auto pushIfDiffInt = [&](const char* key, int val) {
					if (!prefabA || !prefabA->HasMember(key) || differsInt((*prefabA)[key].GetInt(), val))
						ja.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
					};
				auto pushIfDiffFloat = [&](const char* key, float val) {
					if (!prefabA || !prefabA->HasMember(key) || differsFloat((*prefabA)[key].GetFloat(), val))
						ja.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
					};
				auto pushIfDiffBool = [&](const char* key, bool val) {
					if (!prefabA || !prefabA->HasMember(key) ||
						((*prefabA)[key].IsBool() && differsBool((*prefabA)[key].GetBool(), val)) ||
						((*prefabA)[key].IsInt() && differsBool((*prefabA)[key].GetInt() != 0, val))) {
						ja.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
					}
					};

				pushIfDiffBool("loop", an->loop);
				pushIfDiffInt("currentFrameColumn", an->currentFrameColumn);
				pushIfDiffInt("currentFrameRow", an->currentFrameRow);
				pushIfDiffInt("totalColumn", an->totalColumn);
				pushIfDiffInt("totalRow", an->totalRow);
				pushIfDiffFloat("frameTime", an->frameTime);

				if (!ja.ObjectEmpty()) comps.AddMember("Animation", ja, a);
			}

			// ---- StateMachine ----
			if (auto sm = obj->getComponent<StateMachine>()) {
				rapidjson::Value js(rapidjson::kObjectType);
				const rapidjson::Value* prefabSM = PrefabManager::Instance().getPrefabJsonComp(prefabDoc, "StateMachine");

				if (!prefabSM || !prefabSM->HasMember("state") || sm->state != parsePlayerState((*prefabSM)["state"].GetString()))
					js.AddMember("state", rapidjson::Value(playerStateToStr(sm->state), a), a);

				if (!prefabSM || !prefabSM->HasMember("facingRight") ||
					((*prefabSM)["facingRight"].IsBool() && differsBool((*prefabSM)["facingRight"].GetBool(), sm->facingRight)) ||
					((*prefabSM)["facingRight"].IsInt() && differsBool((*prefabSM)["facingRight"].GetInt() != 0, sm->facingRight)))
					js.AddMember("facingRight", rapidjson::Value(sm->facingRight ? 1 : 0), a);

				if (!prefabSM || !prefabSM->HasMember("stateTime") ||
					differsFloat((*prefabSM)["stateTime"].GetFloat(), sm->stateTime))
					js.AddMember("stateTime", rapidjson::Value(sm->stateTime), a);

				if (!js.ObjectEmpty())
					comps.AddMember("StateMachine", js, a);
			}

			//font component
			if (auto fc = obj->getComponent<FontComponent>()) {
				rapidjson::Value jf(rapidjson::kObjectType);

				// word
				jf.AddMember("word",
					rapidjson::Value(fc->word.c_str(), a),
					a);

				// color array
				rapidjson::Value color(rapidjson::kArrayType);
				color.PushBack(rapidjson::Value().SetFloat(fc->clr.r), a)
					.PushBack(rapidjson::Value().SetFloat(fc->clr.g), a)
					.PushBack(rapidjson::Value().SetFloat(fc->clr.b), a);
				jf.AddMember("color", color, a);

				// scale
				jf.AddMember("scale", rapidjson::Value().SetFloat(fc->scale), a);

				// fontType
				jf.AddMember("fontType", rapidjson::Value(fc->fontType), a);

				comps.AddMember("FontComponent", jf, a);
			}

			if (!comps.ObjectEmpty()) jObj.AddMember("components", comps, a);
			objects.PushBack(jObj, a);
		}

		// attach array
		doc.AddMember("objects", objects, a);
	}

	std::string err;
	if (!JsonIO::WriteDocumentToFile(filename, doc, /*pretty=*/true, &err)) {
		std::cerr << "Scene save failed: " << err << "\n";
	}
	else {
		std::cout << "Scene saved (JSON): " << filename << "\n";
	}
}
/*!
\brief Loads a scene automatically based on file extension.

If the path ends with ".json", loads a JSON scene.
Otherwise, falls back to legacy .txt scene loading.

\param path Scene file path.
*/

void GameObjectManager::loadScene(const std::string& path) {
	if (ends_with(path, ".json")) loadSceneFromJson(path);
	else                          loadFromFile(path);   // keep legacy .txt scenes
	//if (GameObject* p = getGameObject("player")) {
	//	if (!p->hasComponent<StateMachine>()) {
	//		p->addComponent<StateMachine>();
	//		std::cout << "[Setup] Added StateMachine to player\n";
	//	}
	//}
}
/*!
\brief Saves a scene automatically based on file extension.

If the path ends with ".json", writes JSON using saveSceneToJson.
Otherwise, writes using legacy .txt serialization.

\param path Output scene path.
\param isNew If true, writes an empty or simplified scene header.
*/

void GameObjectManager::saveScene(const std::string& path, bool isNew) const {
	if (ends_with(path, ".json")) saveSceneToJson(path, isNew);
	else                          saveToFile(path);
}


//layer stuff
LayerManager& GameObjectManager::getLayerManager() {
	return m_layerManager;
}

bool GameObjectManager::assignObjectToLayer(GameObject* obj, int layerID) {
	if (!obj) {
		std::cerr << "GameObjectManager Error: Attempted to assign a null GameObject to layer " << layerID << ".\n";
		return false;
	}

	//just change the id in the object itself
	obj->setLayer(layerID);

	return m_layerManager.assignObjectToLayer(obj, layerID);
}

std::vector<GameObject*> GameObjectManager::getObjectsInLayer(int layerID) {
	Layer* layer = m_layerManager.getLayer(layerID);
	if (layer) {
		return std::vector<GameObject*>(layer->getObjects().begin(), layer->getObjects().end());
	}

	//if layer does not exist, return empty vector
	std::cerr << "GameObjectManager Warning: Layer ID " << layerID << " not found.\n";
	return std::vector<GameObject*>{};
}

void GameObjectManager::initializeSceneResources() {
	// Load textures for render components
	for (auto& pair : m_gameObjects) {
		GameObject* obj = pair.second.get();
		if (obj->hasComponent<Render>()) {
			Render* render = obj->getComponent<Render>();
			if (render->hasTex) {
				TextureData texData = ResourceManager::getInstance().getTexture(render->texFile);
				render->texHDL = texData.id;
				render->isTransparent = texData.isTransparent;
				render->texChanged = false;
			}
		}
		// Initialize font models
		if (obj->hasComponent<FontComponent>()) {
			FontComponent* fc = obj->getComponent<FontComponent>();
			if (!Font::fontMdls.empty()) {
				fc->mdl = Font::fontMdls[0];
			}
		}
	}
}
