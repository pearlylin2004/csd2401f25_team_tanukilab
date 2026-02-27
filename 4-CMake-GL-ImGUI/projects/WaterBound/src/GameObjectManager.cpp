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
#include "Editor/editorState.h"
#include "Editor/undoRedo.h"

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

	const std::string& deletedName = object->getObjectName();

	m_layerManager.removeObjectFromLayer(object);

	auto toErase = m_gameObjects.end();

	for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it) {
		if (it->second.get() == object) {
			toErase = it;
			continue; // don't skip remaining children
		}

		Transform* t = it->second->getComponent<Transform>();
		if (t && t->parentName == deletedName) {
			t->parentName.clear();
			t->localOffset = { 0.f, 0.f };
			t->localRotation = 0.f;
		}
	}

	if (toErase != m_gameObjects.end())
		m_gameObjects.erase(toErase);
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

	//now to do for layer as well
	assignObjectToLayer(ptr, iterator->second->getLayer());

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

int GameObjectManager::getGameObjectCount() {
	return static_cast<int>(m_gameObjects.size());
}

void GameObjectManager::init() {

	//loadScene(JsonIO::runtimeScenePath("level01.json"));
}

bool GameObjectManager::renameGameObject(GameObject* obj, const std::string& newName) {
	if (!obj) return false;
	if (newName.empty()) return false;

	std::string oldName = obj->getObjectName();

	if (m_gameObjects.find(newName) != m_gameObjects.end()) return false;

	auto node = m_gameObjects.extract(oldName);
	if (node.empty()) return false;

	node.key() = newName;
	m_gameObjects.insert(std::move(node));

	// rename if no duplicated name
	obj->getObjectName() = newName;
	return true;

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
	if (s == "Dead")     return PlayerState::Dead;
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
	case PlayerState::Dead:     return "Dead";
	default:                    return "Idle";
	}
}

//inline soundID strToSoundID(const std::string& str) {
//	if (str == "water") return soundID::water;
//	if (str == "shoot") return soundID::shoot;
//	if (str == "fire") return soundID::fire;
//	if (str == "bg") return soundID::bg;
//	return soundID::water; // default fallback
//}
//
//// Add this helper function to convert soundID to string
//inline const char* soundIDToStr(soundID id) {
//	switch (id) {
//		case soundID::water: return "water";
//		case soundID::shoot: return "shoot";
//		case soundID::fire: return "fire";
//		case soundID::bg: return "bg";
//		default: return "water";
//	}
//}

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
	auto t1 = std::chrono::high_resolution_clock::now();

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

	// Clear existing layers and recreate from JSON
	if (doc.HasMember("layers") && doc["layers"].IsArray()) {

	    
	    for (const auto& layerJson : doc["layers"].GetArray()) {
	        if (!layerJson.IsObject()) continue;
	        
	        int layerID = layerJson.HasMember("id") && layerJson["id"].IsInt() 
	            ? layerJson["id"].GetInt() : -1;
	        if (layerID < 0) continue;
	        
	        std::string layerName = layerJson.HasMember("name") && layerJson["name"].IsString()
	            ? layerJson["name"].GetString() 
	            : ("Layer_" + std::to_string(layerID));
	        
	        // Create layer if it doesn't exist
	        Layer* layer = m_layerManager.getLayer(layerID);
	        if (!layer) {
	            layer = m_layerManager.createLayer(layerName, layerID);
	        } else {
	            m_layerManager.renameLayer(layerID, layerName);
	        }
	        
	        if (!layer) continue;
	        
	        // Load flags
	        bool enabled = JsonIO::GetBoolOr(layerJson, "enabled", true);
	        bool visible = JsonIO::GetBoolOr(layerJson, "visible", true);
	        bool updatable = JsonIO::GetBoolOr(layerJson, "updatable", true);
	        
	        layer->setEnabled(enabled);
	        layer->setVisible(visible);
	        layer->setUpdatable(updatable);
	    }
	}
	// ========== END ADD ==========

	if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
		std::cerr << "Scene JSON missing 'objects' array.\n";
		return;
	}

	// prepare template for bullet
	GameObject* templateBullet = nullptr;

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

			/* ---- SCUFFED WAY TO GET BULLETS CLONE ---- */
			// still hardcoded but slightly better than using name only
			// if its bullet prefab, make this obj the template
			if (prefabID == "b1a12273-a692-4ce0-8072-156da2c70842" && !templateBullet) {
				templateBullet = go;
			}
			/* ---- END ---- */
		}
		//else {
		//	// if no prefab, just create a fresh game object
		//	std::string name = (jObj.HasMember("name") && jObj["name"].IsString())
		//		? jObj["name"].GetString()
		//		: ("Object_" + std::to_string(m_gameObjects.size()));
		//}

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

			if (jt.HasMember("parentName") && jt["parentName"].IsString())
				t->parentName = jt["parentName"].GetString();
			if (jt.HasMember("localOffset") && jt["localOffset"].IsArray() && jt["localOffset"].Size() == 2) {
				t->localOffset.x = jt["localOffset"][0].GetFloat();
				t->localOffset.y = jt["localOffset"][1].GetFloat();
			}
			if (jt.HasMember("localRotation") && jt["localRotation"].IsNumber())
				t->localRotation = jt["localRotation"].GetFloat();

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

		if (comps.HasMember("Particle") && comps["Particle"].IsObject()) {
			const auto& jp = comps["Particle"];

			auto* p = go->getComponent<Particle>();
			if (!p) p = go->addComponent<Particle>();

			// isMaster (accept bool or 0/1)
			if (jp.HasMember("isMaster")) {
				p->isMaster = jp["isMaster"].IsBool()
					? jp["isMaster"].GetBool()
					: (jp["isMaster"].GetInt() != 0);
			}
			else {
				p->isMaster = true;
			}

			// Master GroupData - color
			if (jp.HasMember("clr_r")) p->master.clr.r = jp["clr_r"].GetFloat();
			if (jp.HasMember("clr_g")) p->master.clr.g = jp["clr_g"].GetFloat();
			if (jp.HasMember("clr_b")) p->master.clr.b = jp["clr_b"].GetFloat();
			if (jp.HasMember("clr_a")) p->master.clr.a = jp["clr_a"].GetFloat();

			// Master GroupData - velocity
			if (jp.HasMember("velocity_x")) p->master.velocity.x = jp["velocity_x"].GetFloat();
			if (jp.HasMember("velocity_y")) p->master.velocity.y = jp["velocity_y"].GetFloat();

			// Master GroupData - other fields
			if (jp.HasMember("timer")) p->master.timer = jp["timer"].GetFloat();
			if (jp.HasMember("offset")) p->master.offset = jp["offset"].GetFloat();
			if (jp.HasMember("particleAmt")) p->master.particleAmt = jp["particleAmt"].GetInt();

			// Note: isActive is not deserialized as requested
		}

		// ---- Input (presence only) ----
		if (comps.HasMember("Input") && comps["Input"].IsObject()) {
			auto* i = go->getComponent<Input>();
			if (!i) i = go->addComponent<Input>();
		}

		// ---- Physics ----
		if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
			const auto& jp = comps["Physics"];

			auto* p = go->getComponent<Physics>();
			if (!p) p = go->addComponent<Physics>();

			if (jp.HasMember("physicsFlag")) {
				p->physicsFlag = jp["physicsFlag"].IsBool()
					? jp["physicsFlag"].GetBool()
					: (jp["physicsFlag"].GetInt() != 0);
			}

			if (jp.HasMember("autoUpdate")) {
				p->autoUpdate = jp["autoUpdate"].IsBool()
					? jp["autoUpdate"].GetBool()
					: (jp["autoUpdate"].GetInt() != 0);
			}

			if (jp.HasMember("moveSpeed") && jp["moveSpeed"].IsNumber())
				p->moveSpeed = jp["moveSpeed"].GetFloat();

			if (jp.HasMember("jumpForce") && jp["jumpForce"].IsNumber())
				p->jumpForce = jp["jumpForce"].GetFloat();

			if (jp.HasMember("damping") && jp["damping"].IsNumber())
				p->damping = jp["damping"].GetFloat();

			if (jp.HasMember("mass") && jp["mass"].IsNumber())
				p->dynamics.mass = jp["mass"].GetFloat();

			/*if (jp.HasMember("friction") && jp["friction"].IsNumber())
				p->friction = jp["friction"].GetFloat();*/
			if (jp.HasMember("maxFallSpeed") && jp["maxFallSpeed"].IsNumber())
				p->maxFallSpeed = jp["maxFallSpeed"].GetFloat();
			if (jp.HasMember("groundAccelForce") && jp["groundAccelForce"].IsNumber())
				p->groundAccelForce = jp["groundAccelForce"].GetFloat();
			if (jp.HasMember("airAccelForce") && jp["airAccelForce"].IsNumber())
				p->airAccelForce = jp["airAccelForce"].GetFloat();
			if (jp.HasMember("apexHangMul") && jp["apexHangMul"].IsNumber())
				p->apexHangMul = jp["apexHangMul"].GetFloat();
			if (jp.HasMember("jumpBufferTime") && jp["jumpBufferTime"].IsNumber())
				p->jumpBufferTime = jp["jumpBufferTime"].GetFloat();
			if (jp.HasMember("coyoteTime") && jp["coyoteTime"].IsNumber())
				p->coyoteTime = jp["coyoteTime"].GetFloat();
			if (jp.HasMember("ledgeAssistRange") && jp["ledgeAssistRange"].IsNumber())
				p->ledgeAssistRange = jp["ledgeAssistRange"].GetFloat();
			if (jp.HasMember("ledgeClimbSpeed") && jp["ledgeClimbSpeed"].IsNumber())
				p->ledgeClimbSpeed = jp["ledgeClimbSpeed"].GetFloat();


			/*if (jp.HasMember("inWater")) {
				p->inWater = jp["inWater"].IsBool()
					? jp["inWater"].GetBool()
					: (jp["inWater"].GetInt() != 0);
			}*/
			if (jp.HasMember("buoyancy")) {
				p->buoyancy = jp["buoyancy"].IsBool()
					? jp["buoyancy"].GetBool()
					: (jp["buoyancy"].GetInt() != 0);
			}

			if (go->hasComponent<Transform>()) {
				Transform* transform = go->getComponent<Transform>();
				p->dynamics.position.x = transform->x;
				p->dynamics.position.y = transform->y;
			}
			if (jp.HasMember("freezeRotation")) {
				p->freezeRotation = jp["freezeRotation"].IsBool()
				? jp["freezeRotation"].GetBool()
				: (jp["freezeRotation"].GetInt() != 0);
			}

			if (jp.HasMember("angularDrag") && jp["angularDrag"].IsNumber())
				p->dynamics.angularDrag = jp["angularDrag"].GetFloat();

			if (jp.HasMember("inertiaOverride") && jp["inertiaOverride"].IsNumber())
				p->inertiaOverride = jp["inertiaOverride"].GetFloat();
		}

		// ---- CollisionInfo (presence only) ----
		if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
			const auto& jc = comps["CollisionInfo"];
			auto* c = go->getComponent<CollisionInfo>();
			if (!c)
				c = go->addComponent<CollisionInfo>();

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

			if (jc.HasMember("collisionRes") && jc["collisionRes"].IsString()) {
				c->collisionRes = JsonIO::StrToCollisionResponseMode(jc["collisionRes"].GetString());
			}

			if (jc.HasMember("colliderSize") && jc["colliderSize"].IsArray() && jc["colliderSize"].Size() == 2)
			{
				c->colliderSize.x = jc["colliderSize"][0].GetFloat();
				c->colliderSize.y = jc["colliderSize"][1].GetFloat();
			}

			// Load Collision Tag
			if (jc.HasMember("collisionTag") && jc["collisionTag"].IsString()) {
				c->collisionTag = jc["collisionTag"].GetString();
			}

			// Load physics material
			if (jc.HasMember("bounciness") && jc["bounciness"].IsNumber())
				c->material.bounciness = jc["bounciness"].GetFloat();
			if (jc.HasMember("friction") && jc["friction"].IsNumber())
				c->material.friction = jc["friction"].GetFloat();
			if (jc.HasMember("colliderType") && jc["colliderType"].IsString())
				c->colliderType = JsonIO::StrToShape(jc["colliderType"].GetString());

			// Load ignore the collision
			if (jc.HasMember("ignoreCollisionsWith") && jc["ignoreCollisionsWith"].IsArray()) {
				c->ignoreCollisionsWith.clear();
				for (const auto& tag : jc["ignoreCollisionsWith"].GetArray()) {
					if (tag.IsString()) {
						c->ignoreCollisionsWith.insert(tag.GetString());
					}
				}
			}
		}

		if (auto* p = go->getComponent<Physics>()) {
			if (auto* c = go->getComponent<CollisionInfo>()) {
				p->recalculateInertia(c->colliderSize, c->colliderType);

			}
		}

		// ---- State Machine ----
		if (comps.HasMember("StateMachine") && comps["StateMachine"].IsObject()) {
			const auto& smj = comps["StateMachine"];

			auto* sm = go->getComponent<StateMachine>();
			if (!sm)
				sm = go->addComponent<StateMachine>();

			if (smj.HasMember("state") && smj["state"].IsString())
				sm->state = PlayerState::Idle;/*parsePlayerState(smj["state"].GetString());*/

			if (smj.HasMember("facingRight")) {
				if (smj["facingRight"].IsBool())       sm->facingRight = smj["facingRight"].GetBool();
				else if (smj["facingRight"].IsInt())   sm->facingRight = (smj["facingRight"].GetInt() != 0);
			}

			sm->fixState = false;

			/*if (smj.HasMember("stateTime") && smj["stateTime"].IsNumber())
				sm->stateTime = static_cast<float>(smj["stateTime"].GetDouble());*/

			/*std::cout << "\n\nReading scene...\n";
			std::cout << "State: " << sm->facingRight << std::endl;
			std::cout << "State Time: " << sm->stateTime << std::endl;*/

			/* ---- Animation based on state ---- */
			// Helper lambda to parse a single animation state
			auto parseAnimState = [](const rapidjson::Value& animJson) -> AnimateState {
				AnimateState state;

				if (animJson.HasMember("texture") && animJson["texture"].IsString())
					state.texFile = animJson["texture"].GetString();

				if (animJson.HasMember("loop")) {
					if (animJson["loop"].IsBool())
						state.loop = animJson["loop"].GetBool();
					else if (animJson["loop"].IsInt())
						state.loop = (animJson["loop"].GetInt() != 0);
				}

				if (animJson.HasMember("play")) {
					if (animJson["play"].IsBool())
						state.play = animJson["play"].GetBool();
					else if (animJson["play"].IsInt())
						state.play = (animJson["play"].GetInt() != 0);
				}

				if (animJson.HasMember("totalColumn") && animJson["totalColumn"].IsInt())
					state.totalColumn = animJson["totalColumn"].GetInt();

				if (animJson.HasMember("totalRow") && animJson["totalRow"].IsInt())
					state.totalRow = animJson["totalRow"].GetInt();

				if (animJson.HasMember("frameTime") && animJson["frameTime"].IsNumber())
					state.frameTime = static_cast<float>(animJson["frameTime"].GetDouble());

				if (animJson.HasMember("initialFramCol") && animJson["initialFramCol"].IsNumber())
					state.initialFrame.x = static_cast<float>(animJson["initialFramCol"].GetDouble());

				if (animJson.HasMember("initialFramRow") && animJson["initialFramRow"].IsNumber())
					state.initialFrame.y = static_cast<float>(animJson["initialFramRow"].GetDouble());

				if (animJson.HasMember("lastFramCol") && animJson["lastFramCol"].IsNumber())
					state.lastFrame.x = static_cast<float>(animJson["lastFramCol"].GetDouble());

				if (animJson.HasMember("lastFramRow") && animJson["lastFramRow"].IsNumber())
					state.lastFrame.y = static_cast<float>(animJson["lastFramRow"].GetDouble());

				return state;
				};

			// parse each state's animation in the order: Idle, Walking, Jumping, Falling, Shooting
			// matches PlayerState enum order
			std::vector<std::string>stateNames = JsonIO::stateNames;

			for (size_t i = 0; i < stateNames.size(); ++i) {
				if (smj.HasMember(stateNames[i].c_str()) && smj[stateNames[i].c_str()].IsObject()) {
					Animation* anim = go->getComponent<Animation>();
					if (!anim) {
						anim = go->addComponent<Animation>();
						anim->animState.resize(stateNames.size());
					}

					const rapidjson::Value& stateObj = smj[stateNames[i].c_str()];

					if (stateObj.HasMember("animState") && stateObj["animState"].IsObject()) {
						AnimateState parsed = parseAnimState(stateObj["animState"]);
						anim->animState[i] = parsed;
					}
				}
			}
		}

		//font component
		if (comps.HasMember("FontComponent") && comps["FontComponent"].IsObject()) {
			const auto& jf = comps["FontComponent"];

			auto* fc = go->getComponent<FontComponent>();
			if (!fc)
				fc = go->addComponent<FontComponent>();

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

		if (comps.HasMember("AudioComponent") && comps["AudioComponent"].IsObject()) {
			const auto& ja = comps["AudioComponent"];

			auto* audio = go->getComponent<AudioComponent>();
			if (!audio) {
				audio = go->addComponent<AudioComponent>();
			}

			/*use chJson because ja is the top level of audio component*/
			//Channels
			if (ja.HasMember("channels") && ja["channels"].IsObject()) {
				const auto& channelsJson = ja["channels"];
				for (auto it = channelsJson.MemberBegin(); it != channelsJson.MemberEnd(); ++it) {
					std::string channelName = it->name.GetString();
					const auto& chJson = it->value;

					AudioChannel* ch = audio->getOrCreateChannel(channelName);

					// Load sound ID from string
					if (chJson.HasMember("audioFile") && chJson["audioFile"].IsString()) {
						ch->audioFile = chJson["audioFile"].GetString();
						std::cout << "Loaded audio file for channel '" << channelName << "': " << ch->audioFile << std::endl;
					}

					// Volume
					if (chJson.HasMember("volume") && chJson["volume"].IsNumber()) {
						ch->volume = chJson["volume"].GetFloat();
					}

					// Pitch
					if (chJson.HasMember("pitch") && chJson["pitch"].IsNumber()) {
					 ch->pitch = chJson["pitch"].GetFloat();
					}

					// Loop
					if (chJson.HasMember("loop")) {
						ch->loop = chJson["loop"].IsBool()
							? chJson["loop"].GetBool()
							: (chJson["loop"].GetInt() != 0);
					}

					// Play on start
					if (chJson.HasMember("playOnStart")) {
					 ch->playOnStart = chJson["playOnStart"].IsBool()
							? chJson["playOnStart"].GetBool()
							: (chJson["playOnStart"].GetInt() != 0);
					}

					// Muted
					if (chJson.HasMember("muted")) {
						ch->muted = chJson["muted"].IsBool()
							? chJson["muted"].GetBool()
							: (chJson["muted"].GetInt() != 0);
					}

					// Fade settings
					if (chJson.HasMember("fadeInOnStart")) {
						ch->fadeInOnStart = chJson["fadeInOnStart"].IsBool()
							? chJson["fadeInOnStart"].GetBool()
							: (chJson["fadeInOnStart"].GetInt() != 0);
					}

					//fade in duration 
					if (chJson.HasMember("fadeInDuration") && chJson["fadeInDuration"].IsNumber()) {
						ch->fadeInDuration = chJson["fadeInDuration"].GetFloat();
					}

					//fade out on stop
					if (chJson.HasMember("fadeOutOnStop")) {
						ch->fadeOutOnStop = chJson["fadeOutOnStop"].IsBool()
							? chJson["fadeOutOnStop"].GetBool()
							: (chJson["fadeOutOnStop"].GetInt() != 0);
					}

					// fade out duration
					if (chJson.HasMember("fadeOutDuration") && chJson["fadeOutDuration"].IsNumber()) {
						ch->fadeOutDuration = chJson["fadeOutDuration"].GetFloat();
					}

					// Load 3D audio settings
					if (chJson.HasMember("isSpatial")) {
						ch->isSpatial = chJson["isSpatial"].IsBool()
							? chJson["isSpatial"].GetBool()
							: (chJson["isSpatial"].GetInt() != 0); 
					}
					if (chJson.HasMember("minDistance") && chJson["minDistance"].IsNumber()) {
						ch->minDistance = chJson["minDistance"].GetFloat();
					}
					if (chJson.HasMember("maxDistance") && chJson["maxDistance"].IsNumber()) {
						ch->maxDistance = chJson["maxDistance"].GetFloat();
					}

				}
			}
		}

		// ---- TileMap ----
		if (comps.HasMember("TileMap") && comps["TileMap"].IsObject()) {
			const auto& tmj = comps["TileMap"];
			TileMap* tm = go->getComponent<TileMap>();
			if (!tm) tm = go->addComponent<TileMap>();

			if (tmj.HasMember("tileW") && tmj["tileW"].IsNumber())
				tm->tileW = tmj["tileW"].GetFloat();

			if (tmj.HasMember("tileH") && tmj["tileH"].IsNumber())
				tm->tileH = tmj["tileH"].GetFloat();

			if (tmj.HasMember("columns") && tmj["columns"].IsInt())
				tm->columns = tmj["columns"].GetInt();

			if (tmj.HasMember("rows") && tmj["rows"].IsInt())
				tm->rows = tmj["rows"].GetInt();

			if (tmj.HasMember("enableCol")) {
				tm->enableCol = tmj["enableCol"].IsBool()
					? tmj["enableCol"].GetBool()
					: (tmj["enableCol"].GetInt() != 0);
			}

			tm->tiles.clear();

			if (tmj.HasMember("tiles") && tmj["tiles"].IsArray()) {
				for (const auto& t : tmj["tiles"].GetArray()) {
					if (!t.IsObject()) continue;

					int x = t["x"].GetInt();
					int y = t["y"].GetInt();
					std::string id = t["id"].GetString();

					tm->setTile(x, y, id);
				}
			}
		}

		// ---- Lua Script ----
		if (comps.HasMember("LuaScript") && comps["LuaScript"].IsObject()) {
			const auto& jl = comps["LuaScript"];

			auto* ls = go->getComponent<LuaScript>();
			if (!ls) ls = go->addComponent<LuaScript>();

			ls->scripts.clear();

			if (jl.HasMember("scripts") && jl["scripts"].IsArray()) {
				for (const auto& scriptEntry : jl["scripts"].GetArray()) {
					if (!scriptEntry.IsObject()) continue;

					LuaScript::ScriptEntry entry;

					if (scriptEntry.HasMember("filename") && scriptEntry["filename"].IsString()) {
						entry.filename = scriptEntry["filename"].GetString();
					}

					if (scriptEntry.HasMember("scriptName") && scriptEntry["scriptName"].IsString()) {
						entry.scriptName = scriptEntry["scriptName"].GetString();
					}

					entry.changed = true;
					ls->scripts.push_back(entry);
				}
			}

			ls->scriptsChanged = true;
		}

		// ---- Visual Logic Component ----
		if (comps.HasMember("VisualLC") && comps["VisualLC"].IsObject()) {
			const auto& jvl = comps["VisualLC"];

			auto* vl = go->getComponent<VisualLC>();
			if (!vl) vl = go->addComponent<VisualLC>();

			// load filename
			if (jvl.HasMember("filename") && jvl["filename"].IsString()) {
				vl->filename = jvl["filename"].GetString();

				// load visual logic from file
				if (!vl->filename.empty()) {
					std::string filepath = "/Logic/" + vl->filename;
					std::string e;
					if (!JsonIO::loadVisualLogic(filepath, vl->nodes, vl->connections, &e)) {
						DebugLog::addMessage("Failed to load Visual Logic file: " + e);
					}
				}
			}
		}

		// ---- Button ----
		if (comps.HasMember("Button") && comps["Button"].IsObject()) {
			const auto& jb = comps["Button"];

			//add Button component if it doesn't exist
			auto* b = go->getComponent<Button>();
			if (!b)
				b = go->addComponent<Button>();

			//default to NONE
			b->actionName = ButtonAction::NONE;

			//parse action string
			if (jb.HasMember("action") && jb["action"].IsString()) {
				std::string actionStr = jb["action"].GetString();
				b->actionName = JsonIO::StrToButtonAction(actionStr);
			}
		}
		// ---- WaterBody ----
		if (comps.HasMember("WaterBody") && comps["WaterBody"].IsObject()) {
			const auto& jw = comps["WaterBody"];

			auto* wb = go->getComponent<WaterBody>();
			if (!wb)
				wb = go->addComponent<WaterBody>();

			// Wave animation parameters
			if (jw.HasMember("waveAmplitude") && jw["waveAmplitude"].IsNumber())
				wb->waveAmplitude = jw["waveAmplitude"].GetFloat();

			if (jw.HasMember("waveFrequency") && jw["waveFrequency"].IsNumber())
				wb->waveFrequency = jw["waveFrequency"].GetFloat();

			if (jw.HasMember("waveSpeed") && jw["waveSpeed"].IsNumber())
				wb->waveSpeed = jw["waveSpeed"].GetFloat();

			// Water visual properties
			if (jw.HasMember("waterColor") && jw["waterColor"].IsArray() && jw["waterColor"].Size() == 4) {
				wb->waterColor.r = jw["waterColor"][0].GetFloat();
				wb->waterColor.g = jw["waterColor"][1].GetFloat();
				wb->waterColor.b = jw["waterColor"][2].GetFloat();
				wb->waterColor.a = jw["waterColor"][3].GetFloat();
			}

			if (jw.HasMember("waterAlpha") && jw["waterAlpha"].IsNumber())
				wb->waterAlpha = jw["waterAlpha"].GetFloat();

			// Subdivision for wave mesh
			if (jw.HasMember("subdivisions") && jw["subdivisions"].IsInt())
				wb->subdivisions = jw["subdivisions"].GetInt();

			// Water body dimensions
			if (jw.HasMember("width") && jw["width"].IsNumber())
				wb->width = jw["width"].GetFloat();

			if (jw.HasMember("height") && jw["height"].IsNumber())
				wb->height = jw["height"].GetFloat();

			// Physics properties
			if (jw.HasMember("isTrigger")) {
				wb->isTrigger = jw["isTrigger"].IsBool()
					? jw["isTrigger"].GetBool()
					: (jw["isTrigger"].GetInt() != 0);
			}

			if (jw.HasMember("buoyancyForce") && jw["buoyancyForce"].IsNumber())
				wb->buoyancyForce = jw["buoyancyForce"].GetFloat();

			if (jw.HasMember("waterDrag") && jw["waterDrag"].IsNumber())
				wb->waterDrag = jw["waterDrag"].GetFloat();

			// Rendering flags
			if (jw.HasMember("renderWaves")) {
				wb->renderWaves = jw["renderWaves"].IsBool()
					? jw["renderWaves"].GetBool()
					: (jw["renderWaves"].GetInt() != 0);
			}

			if (jw.HasMember("useTexture")) {
				wb->useTexture = jw["useTexture"].IsBool()
					? jw["useTexture"].GetBool()
					: (jw["useTexture"].GetInt() != 0);
			}
			/*if (wb->useTexture && jw.HasMember("waterTextureFile") && jw["waterTextureFile"].IsString()) {
				std::string texFile = jw["waterTextureFile"].GetString();
				TextureData texData = ResourceManager::getInstance().getTexture(texFile);
				wb->waterTexture = texData.id;
			}*/

		}
		// ---- WeaponComponent ----
		if (comps.HasMember("WeaponComponent") && comps["WeaponComponent"].IsObject()) {
			const auto& jweap = comps["WeaponComponent"];

			auto* wc = go->getComponent<WeaponComponent>();
			if (!wc)
				wc = go->addComponent<WeaponComponent>();

			if (jweap.HasMember("barrelOffset") && jweap["barrelOffset"].IsArray()
				&& jweap["barrelOffset"].Size() >= 2) {
				wc->barrelOffset.x = jweap["barrelOffset"][0].GetFloat();
				wc->barrelOffset.y = jweap["barrelOffset"][1].GetFloat();
			}

			if (jweap.HasMember("aimAtMouse")) {
				wc->aimAtMouse = jweap["aimAtMouse"].IsBool()
					? jweap["aimAtMouse"].GetBool()
					: (jweap["aimAtMouse"].GetInt() != 0);
			}
		}
	}

	//create bullet pool
	/* ---- SCUFFED WAY TO GET BULLET WORK ---- */
	if (!templateBullet) templateBullet = getGameObject("bullet");

	if (templateBullet) {
		createBulletPool(templateBullet->getObjectName(), 10);
	}
	/* ---- END ---- */

	std::cout << "Total game objects loaded: " << m_gameObjects.size() << std::endl;

	//std::cout << "Scene loaded (JSON): " << filename << "\n";

	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "[Timing] Loading json took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " ms" << std::endl;
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

		rapidjson::Value layersArr(rapidjson::kArrayType);

for (Layer* layer : m_layerManager.getAllLayers()) {
    if (!layer) continue;

    rapidjson::Value layerObj(rapidjson::kObjectType);
    layerObj.AddMember("id", layer->getLayerID(), a);
    layerObj.AddMember("name", rapidjson::Value(layer->getLayerName().c_str(), a), a);
    layerObj.AddMember("enabled", layer->isEnabled() ? 1 : 0, a);  // Use 1/0 for consistency
    layerObj.AddMember("visible", layer->isVisible() ? 1 : 0, a);
    layerObj.AddMember("updatable", layer->isUpdatable() ? 1 : 0, a);

    layersArr.PushBack(layerObj, a);
}

doc.AddMember("layers", layersArr, a);
	
		bool bulletSerialized = false;
		// objects array
		rapidjson::Value objects(rapidjson::kArrayType);

		for (auto const& kv : m_gameObjects) {
			const GameObject* obj = kv.second.get();

			std::string objName = obj->getObjectName();

			// if its a clone of bullet, serialize only the first one
			if (obj->getObjectPrefabID() == "b1a12273-a692-4ce0-8072-156da2c70842") {
				// if its a bullet, change name to bullet
				if (!bulletSerialized) {
					objName = "bullet";
					bulletSerialized = true;
				}
				else {
					continue;
				}
			}

			rapidjson::Value jObj(rapidjson::kObjectType);

			// prefab ID
			const std::string& prefabID = obj->getObjectPrefabID();
			if (!prefabID.empty()) {
				jObj.AddMember("prefabid", rapidjson::Value(prefabID.c_str(), a), a);
			}

			// name
			jObj.AddMember("name", rapidjson::Value(objName.c_str(), a), a);


			// get prefab JSON if obj referencing a prefab
			const rapidjson::Document* prefabDoc{ nullptr };
			if (!prefabID.empty()) {
				prefabDoc = PrefabManager::Instance().getPrefabJson(prefabID);
			}

			int layerID = obj->getLayer();
			const rapidjson::Value* prefabLayer = PM.getPrefabJsonComp(prefabDoc, "layer");
			if (layerID != -1 && layerID != 1 && (!prefabLayer || !prefabLayer->IsInt() || prefabLayer->GetInt() != layerID)) {  // Only save if not default game layer
				jObj.AddMember("layer", layerID, a);
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

				// Save parent-child hierarchy (only if have parent)
				if (!t->parentName.empty()) {
					jt.AddMember("parentName", rapidjson::Value(t->parentName.c_str(), a), a);
					rapidjson::Value lo(rapidjson::kArrayType);
					lo.PushBack(t->localOffset.x, a).PushBack(t->localOffset.y, a);
					jt.AddMember("localOffset", lo, a);
					jt.AddMember("localRotation", t->localRotation, a);
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

			// ---- Particle ----
			if (auto p = obj->getComponent<Particle>()) {
				// Only serialize if this is a master particle
				if (p->isMaster) {
					rapidjson::Value jp(rapidjson::kObjectType);
					const rapidjson::Value* prefabP = PM.getPrefabJsonComp(prefabDoc, "Particle");

					auto pushIfDiffBool = [&](const char* key, bool val) {
						if (!prefabP || !prefabP->HasMember(key) ||
							((*prefabP)[key].IsBool() && differsBool((*prefabP)[key].GetBool(), val)) ||
							((*prefabP)[key].IsInt() && differsBool((*prefabP)[key].GetInt() != 0, val))) {
							jp.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
						}
						};

					auto pushIfDiffFloat = [&](const char* key, float val) {
						if (!prefabP || !prefabP->HasMember(key) ||
							!(*prefabP)[key].IsNumber() ||
							((*prefabP)[key].GetFloat() != val))
						{
							jp.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
						}
						};

					auto pushIfDiffInt = [&](const char* key, int val) {
						if (!prefabP || !prefabP->HasMember(key) ||
							!(*prefabP)[key].IsNumber() ||
							((*prefabP)[key].GetInt() != val))
						{
							jp.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
						}
						};

					// Serialize master data
					pushIfDiffBool("isMaster", p->isMaster);

					// Master GroupData
					pushIfDiffFloat("clr_r", p->master.clr.r);
					pushIfDiffFloat("clr_g", p->master.clr.g);
					pushIfDiffFloat("clr_b", p->master.clr.b);
					pushIfDiffFloat("clr_a", p->master.clr.a);
					pushIfDiffFloat("velocity_x", p->master.velocity.x);
					pushIfDiffFloat("velocity_y", p->master.velocity.y);
					pushIfDiffFloat("timer", p->master.timer);
					pushIfDiffFloat("offset", p->master.offset);
					pushIfDiffInt("particleAmt", p->master.particleAmt);

					if (!jp.ObjectEmpty()) comps.AddMember("Particle", jp, a);
				}
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

				auto pushIfDiffFloat = [&](const char* key, float val) {
					if (!prefabP || !prefabP->HasMember(key) ||
						!(*prefabP)[key].IsNumber() ||
						((*prefabP)[key].GetFloat() != val))
					{
						jp.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
					}
					};

				// if different then change scene data
				pushIfDiffBool("physicsFlag", p->physicsFlag);
				//pushIfDiffBool("inWater", p->inWater);
				pushIfDiffBool("autoUpdate", p->autoUpdate);
				pushIfDiffBool("buoyancy", p->buoyancy);

				pushIfDiffFloat("moveSpeed", p->moveSpeed);
				pushIfDiffFloat("jumpForce", p->jumpForce);
				pushIfDiffFloat("mass", p->dynamics.mass);
				pushIfDiffFloat("damping", p->damping);
				//pushIfDiffFloat("friction", p->friction);
				pushIfDiffFloat("maxFallSpeed", p->maxFallSpeed);
				pushIfDiffFloat("groundAccelForce", p->groundAccelForce);
				pushIfDiffFloat("airAccelForce", p->airAccelForce);
				pushIfDiffFloat("apexHangMul", p->apexHangMul);
				pushIfDiffFloat("jumpBufferTime", p->jumpBufferTime);
				pushIfDiffFloat("coyoteTime", p->coyoteTime);
				pushIfDiffFloat("ledgeAssistRange", p->ledgeAssistRange);
				pushIfDiffFloat("ledgeClimbSpeed", p->ledgeClimbSpeed);

				pushIfDiffBool("freezeRotation", p->freezeRotation);
				pushIfDiffFloat("angularDrag", p->dynamics.angularDrag);
				pushIfDiffFloat("inertiaOverride", p->inertiaOverride);

				if (!jp.ObjectEmpty()) comps.AddMember("Physics", jp, a);
			}

			// ---- Input (presence only) ----
			if (obj->hasComponent<Input>()) {
				const rapidjson::Value* prefabI = PM.getPrefabJsonComp(prefabDoc, "Input");
				if (!prefabI) comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
			}

			// ---- CollisionInfo (presence only) ----
			if (auto c = obj->getComponent<CollisionInfo>()) {
				rapidjson::Value jc(rapidjson::kObjectType);
				const rapidjson::Value* prefabC = PM.getPrefabJsonComp(prefabDoc, "CollisionInfo");

				auto pushIfDiffBool = [&](const char* key, bool val) {
					if (!prefabC || !prefabC->HasMember(key) ||
						((*prefabC)[key].IsBool() && differsBool((*prefabC)[key].GetBool(), val)) ||
						((*prefabC)[key].IsInt() && differsBool((*prefabC)[key].GetInt() != 0, val))) {
						jc.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
					}
					};

				pushIfDiffBool("collisionFlag", c->collisionFlag);
				pushIfDiffBool("autoFitScale", c->autoFitScale);

				if (!prefabC || !prefabC->HasMember("colliderSize") ||
					differsFloat((*prefabC)["colliderSize"][0].GetFloat(), c->colliderSize.x) ||
					differsFloat((*prefabC)["colliderSize"][1].GetFloat(), c->colliderSize.y)) {

					rapidjson::Value sc(rapidjson::kArrayType);
					sc.PushBack(c->colliderSize.x, a).PushBack(c->colliderSize.y, a);
					jc.AddMember("colliderSize", sc, a);
				}

				std::string shapeStr = JsonIO::ShapeToStr(c->colliderType);
				if (!prefabC || !prefabC->HasMember("colliderType") || shapeStr != (*prefabC)["colliderType"].GetString())
					jc.AddMember("colliderType", rapidjson::Value(shapeStr.c_str(), a), a);

				std::string collisionResStr = JsonIO::CollisionResponseModeToStr(c->collisionRes);
				if (!prefabC || !prefabC->HasMember("collisionRes") || collisionResStr != (*prefabC)["collisionRes"].GetString()) {
					jc.AddMember("collisionRes", rapidjson::Value(collisionResStr.c_str(), a), a);
				}

				if (!prefabC || !prefabC->HasMember("collisionTag") ||
					c->collisionTag != (*prefabC)["collisionTag"].GetString()) {
					jc.AddMember("collisionTag", rapidjson::Value(c->collisionTag.c_str(), a), a);
				}

				// Save physics material (only if differs from prefab or no prefab)
				if (!prefabC || !prefabC->HasMember("bounciness") ||
					differsFloat((*prefabC)["bounciness"].GetFloat(), c->material.bounciness))
					jc.AddMember("bounciness", c->material.bounciness, a);

				if (!prefabC || !prefabC->HasMember("friction") ||
					differsFloat((*prefabC)["friction"].GetFloat(), c->material.friction))
					jc.AddMember("friction", c->material.friction, a);
				std::string colliderTypeStr = JsonIO::ShapeToStr(c->colliderType);
				if (!prefabC || !prefabC->HasMember("colliderType") || colliderTypeStr != (*prefabC)["colliderType"].GetString()) {
					jc.AddMember("colliderType", rapidjson::Value(colliderTypeStr.c_str(), a), a);
				}

				bool shouldSaveIgnoreList = false;
				if (!prefabC || !prefabC->HasMember("ignoreCollisionsWith")) {
					shouldSaveIgnoreList = !c->ignoreCollisionsWith.empty();
				}
				else {
					// compare with prefab ignore list
					const rapidjson::Value& prefabIgnoreList = (*prefabC)["ignoreCollisionsWith"];
					if (prefabIgnoreList.Size() != c->ignoreCollisionsWith.size()) {
						shouldSaveIgnoreList = true;
					}
					else {
						// check if all elements match
						std::set<std::string> prefabSet;
						for (rapidjson::SizeType i = 0; i < prefabIgnoreList.Size(); ++i) {
							prefabSet.insert(prefabIgnoreList[i].GetString());
						}
						for (const std::string& tag : c->ignoreCollisionsWith) {
							if (prefabSet.find(tag) == prefabSet.end()) {
								shouldSaveIgnoreList = true;
								break;
							}
						}
					}
				}

				if (shouldSaveIgnoreList) {
					rapidjson::Value ignoreArray(rapidjson::kArrayType);
					for (const std::string& tag : c->ignoreCollisionsWith) {
						ignoreArray.PushBack(rapidjson::Value(tag.c_str(), a), a);
					}
					jc.AddMember("ignoreCollisionsWith", ignoreArray, a);
				}

				if (!jc.ObjectEmpty()) {
					comps.AddMember("CollisionInfo", jc, a);
				}
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

				/*if (!prefabSM || !prefabSM->HasMember("stateTime") ||
					differsFloat((*prefabSM)["stateTime"].GetFloat(), sm->stateTime))
					js.AddMember("stateTime", rapidjson::Value(sm->stateTime), a);*/


				/* ------ Animation for states ----- */
				if (auto anim = obj->getComponent<Animation>()) {
					std::vector<std::string> stateNames = JsonIO::stateNames;

					for (size_t i = 0; i < stateNames.size() && i < anim->animState.size(); ++i) {
						const AnimateState& animState = anim->animState[i];

						// check if this state differs from prefab
						bool shouldSerialize = false;
						const rapidjson::Value* prefabStateObj = nullptr;

						if (prefabSM && prefabSM->HasMember(stateNames[i].c_str())) {
							prefabStateObj = &(*prefabSM)[stateNames[i].c_str()];
							if (prefabStateObj->IsObject() && prefabStateObj->HasMember("animState")) {
								const auto& prefabAnimState = (*prefabStateObj)["animState"];

								shouldSerialize =
									(!prefabAnimState.HasMember("texture") || animState.texFile != prefabAnimState["texture"].GetString()) ||
									(!prefabAnimState.HasMember("loop") || animState.loop != (prefabAnimState["loop"].IsBool() ? prefabAnimState["loop"].GetBool() : (prefabAnimState["loop"].GetInt() != 0))) ||
									(!prefabAnimState.HasMember("play") || animState.play != (prefabAnimState["play"].IsBool() ? prefabAnimState["play"].GetBool() : (prefabAnimState["play"].GetInt() != 0))) ||
									(!prefabAnimState.HasMember("totalColumn") || animState.totalColumn != prefabAnimState["totalColumn"].GetInt()) ||
									(!prefabAnimState.HasMember("totalRow") || animState.totalRow != prefabAnimState["totalRow"].GetInt()) ||
									(!prefabAnimState.HasMember("frameTime") || differsFloat(static_cast<float>(prefabAnimState["frameTime"].GetDouble()), animState.frameTime));
								(!prefabAnimState.HasMember("initialFramCol") || differsFloat(static_cast<float>(prefabAnimState["initialFramCol"].GetDouble()), animState.initialFrame.x));
								(!prefabAnimState.HasMember("initialFramRow") || differsFloat(static_cast<float>(prefabAnimState["initialFramRow"].GetDouble()), animState.initialFrame.y));
								(!prefabAnimState.HasMember("lastFramCol") || differsFloat(static_cast<float>(prefabAnimState["lastFramCol"].GetDouble()), animState.lastFrame.x));
								(!prefabAnimState.HasMember("lastFramRow") || differsFloat(static_cast<float>(prefabAnimState["lastFramRow"].GetDouble()), animState.lastFrame.y));
							}
							else {
								shouldSerialize = true; // prefab doesn't have animState for this state
							}
						}
						else {
							shouldSerialize = true; // prefab dont this this
						}

						// only serialize if different from prefab or if no prefab
						if (shouldSerialize) {
							rapidjson::Value stateObj(rapidjson::kObjectType);
							rapidjson::Value animStateObj(rapidjson::kObjectType);

							animStateObj.AddMember("texture", rapidjson::Value(animState.texFile.c_str(), a), a);
							animStateObj.AddMember("loop", rapidjson::Value(animState.loop ? 1 : 0), a);
							animStateObj.AddMember("play", rapidjson::Value(animState.play ? 1 : 0), a);
							animStateObj.AddMember("totalColumn", rapidjson::Value(animState.totalColumn), a);
							animStateObj.AddMember("totalRow", rapidjson::Value(animState.totalRow), a);
							animStateObj.AddMember("frameTime", rapidjson::Value(animState.frameTime), a);
							animStateObj.AddMember("initialFramCol", rapidjson::Value(animState.initialFrame.x), a);
							animStateObj.AddMember("initialFramRow", rapidjson::Value(animState.initialFrame.y), a);
							animStateObj.AddMember("lastFramCol", rapidjson::Value(animState.lastFrame.x), a);
							animStateObj.AddMember("lastFramRow", rapidjson::Value(animState.lastFrame.y), a);

							stateObj.AddMember("animState", animStateObj, a);
							js.AddMember(rapidjson::Value(stateNames[i].c_str(), a), stateObj, a);
						}
					}
				}
				/* ---------- END ---------- */


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

			//audio
			if (auto ac = obj->getComponent<AudioComponent>()) {
				rapidjson::Value jac(rapidjson::kObjectType);
				rapidjson::Value channelsOverride(rapidjson::kObjectType);
				const rapidjson::Value* prefabAC = PM.getPrefabJsonComp(prefabDoc, "AudioComponent");

				// Get prefab channels for comparison (if exists)
				const rapidjson::Value* prefabChannels = nullptr;
				if (prefabAC && prefabAC->HasMember("channels") && (*prefabAC)["channels"].IsObject()) {
					prefabChannels = &(*prefabAC)["channels"];
				}

				// Check ALL channels for overrides
				for (const auto& [channelName, channel] : ac->audioChannels) {
					rapidjson::Value channelOverride(rapidjson::kObjectType);

					// Get prefab version of this channel (if exists)
					const rapidjson::Value* prefabChannel = nullptr;
					if (prefabChannels && prefabChannels->HasMember(channelName.c_str())) {
						prefabChannel = &(*prefabChannels)[channelName.c_str()];
					}

					// Helper lambdas for this channel
					auto pushIfDiffStr = [&](const char* key, const char* val) {
						if (!prefabChannel || !prefabChannel->HasMember(key) ||
							std::strcmp((*prefabChannel)[key].GetString(), val) != 0) {
							channelOverride.AddMember(rapidjson::Value(key, a),
								rapidjson::Value(val, a), a);
						}
						};

					auto pushIfDiffFloat = [&](const char* key, float val) {
						if (!prefabChannel || !prefabChannel->HasMember(key) ||
							differsFloat((*prefabChannel)[key].GetFloat(), val)) {
							channelOverride.AddMember(rapidjson::Value(key, a),
								rapidjson::Value(val), a);
						}
						};

					auto pushIfDiffBool = [&](const char* key, bool val) {
						if (!prefabChannel || !prefabChannel->HasMember(key) ||
							((*prefabChannel)[key].IsBool() && differsBool((*prefabChannel)[key].GetBool(), val)) ||
							((*prefabChannel)[key].IsInt() && differsBool((*prefabChannel)[key].GetInt() != 0, val))) {
							channelOverride.AddMember(rapidjson::Value(key, a),
								rapidjson::Value(val ? 1 : 0), a);
						}
						};

					// Check each property for differences
					pushIfDiffStr("audioFile", channel.audioFile.c_str());
					pushIfDiffFloat("volume", channel.volume);
					pushIfDiffFloat("pitch", channel.pitch);
					pushIfDiffBool("loop", channel.loop);
					pushIfDiffBool("playOnStart", channel.playOnStart);
					pushIfDiffBool("muted", channel.muted);
					pushIfDiffBool("fadeInOnStart", channel.fadeInOnStart);
					pushIfDiffFloat("fadeInDuration", channel.fadeInDuration);
					pushIfDiffBool("fadeOutOnStop", channel.fadeOutOnStop);
					pushIfDiffFloat("fadeOutDuration", channel.fadeOutDuration);

					// Check 3D audio overrides
					pushIfDiffBool("isSpatial", channel.isSpatial);
					pushIfDiffFloat("minDistance", channel.minDistance);
					pushIfDiffFloat("maxDistance", channel.maxDistance);

					// If this channel has overrides OR doesn't exist in prefab, save it
					if (!channelOverride.ObjectEmpty() || !prefabChannel) {
						channelsOverride.AddMember(
							rapidjson::Value(channelName.c_str(), a),
							channelOverride,
							a
						);
					}
				}

				// Only save AudioComponent if there are channel overrides
				if (!channelsOverride.ObjectEmpty()) {
					jac.AddMember("channels", channelsOverride, a);
					comps.AddMember("AudioComponent", jac, a);
				}
			}

			// ---- TileMap ----
			if (auto tm = obj->getComponent<TileMap>()) {
				rapidjson::Value jt(rapidjson::kObjectType);
				const rapidjson::Value* prefabTM = PM.getPrefabJsonComp(prefabDoc, "TileMap");

				// compare helper
				auto diffFloat = [&](const char* key, float val) {
					return (!prefabTM || !prefabTM->HasMember(key) ||
						std::fabs(prefabTM->FindMember(key)->value.GetFloat() - val) > 1e-6f);
					};

				auto diffInt = [&](const char* key, int val) {
					return (!prefabTM || !prefabTM->HasMember(key) ||
						prefabTM->FindMember(key)->value.GetInt() != val);
					};

				auto diffBool = [&](const char* key, bool val) {
					if (!prefabTM || !prefabTM->HasMember(key)) return true;
					const auto& v = prefabTM->FindMember(key)->value;
					return (v.IsBool() && v.GetBool() != val) ||
						(v.IsInt() && (v.GetInt() != 0) != val);
				};

				if (diffFloat("tileW", tm->tileW))
					jt.AddMember("tileW", tm->tileW, a);

				if (diffFloat("tileH", tm->tileH))
					jt.AddMember("tileH", tm->tileH, a);

				if (diffInt("columns", tm->columns))
					jt.AddMember("columns", tm->columns, a);

				if (diffInt("rows", tm->rows))
					jt.AddMember("rows", tm->rows, a);

				if (diffBool("enableCol", tm->enableCol))
					jt.AddMember("enableCol", rapidjson::Value(tm->enableCol ? 1 : 0), a);

				// Tiles array
				{
					rapidjson::Value tilesArr(rapidjson::kArrayType);

					for (const auto& [key, id] : tm->tiles) {
						rapidjson::Value t(rapidjson::kObjectType);
						t.AddMember("x", key.x, a);
						t.AddMember("y", key.y, a);
						t.AddMember("id", rapidjson::Value(id.c_str(), a), a);
						tilesArr.PushBack(t, a);
					}

					jt.AddMember("tiles", tilesArr, a);
				}

				if (!jt.ObjectEmpty())
					comps.AddMember("TileMap", jt, a);
			}

			// ---- LuaScript ----
			if (auto ls = obj->getComponent<LuaScript>()) {
				rapidjson::Value jl(rapidjson::kObjectType);
				PM.getPrefabJsonComp(prefabDoc, "LuaScript");

				rapidjson::Value scriptsArray(rapidjson::kArrayType);

				for (const auto& scriptEntry : ls->scripts) {
					if (scriptEntry.filename.empty() && scriptEntry.scriptName.empty()) continue;

					rapidjson::Value entry(rapidjson::kObjectType);
					entry.AddMember("filename", rapidjson::Value(scriptEntry.filename.c_str(), a), a);
					entry.AddMember("scriptName", rapidjson::Value(scriptEntry.scriptName.c_str(), a), a);

					scriptsArray.PushBack(entry, a);
				}

				if (!scriptsArray.Empty()) {
					jl.AddMember("scripts", scriptsArray, a);
					comps.AddMember("LuaScript", jl, a);
				}
			}

			// ---- Visual Logic ----
			if (auto vl = obj->getComponent<VisualLC>()) {
				rapidjson::Value jvl(rapidjson::kObjectType);
				const rapidjson::Value* prefabVL = PM.getPrefabJsonComp(prefabDoc, "VisualLC");

					// save only if filename dff from prefab or prefab no have
		if (!vl->filename.empty()) {
			if (!prefabVL || !prefabVL->HasMember("filename") ||
				vl->filename != (*prefabVL)["filename"].GetString()) {
				jvl.AddMember("filename", rapidjson::Value(vl->filename.c_str(), a), a);
			}
		}

		if (!jvl.ObjectEmpty()) {
			comps.AddMember("VisualLC", jvl, a);
		}
		}
		// ---- WaterBody ---- 
		if (auto wb = obj->getComponent<WaterBody>()) {
			rapidjson::Value jw(rapidjson::kObjectType);
			const rapidjson::Value* prefabWB = PM.getPrefabJsonComp(prefabDoc, "WaterBody");

			// Helper lambda for comparing floats
			auto pushIfDiffFloat = [&](const char* key, float val) {
				if (!prefabWB || !prefabWB->HasMember(key) ||
					!(*prefabWB)[key].IsNumber() ||
					differsFloat((*prefabWB)[key].GetFloat(), val)) {
					jw.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
				}
				};

			auto pushIfDiffInt = [&](const char* key, int val) {
				if (!prefabWB || !prefabWB->HasMember(key) ||
					!(*prefabWB)[key].IsInt() ||
					differsInt((*prefabWB)[key].GetInt(), val)) {
					jw.AddMember(rapidjson::Value(key, a), rapidjson::Value(val), a);
				}
				};

			auto pushIfDiffBool = [&](const char* key, bool val) {
				if (!prefabWB || !prefabWB->HasMember(key) ||
					((*prefabWB)[key].IsBool() && differsBool((*prefabWB)[key].GetBool(), val)) ||
					((*prefabWB)[key].IsInt() && differsBool((*prefabWB)[key].GetInt() != 0, val))) {
					jw.AddMember(rapidjson::Value(key, a), rapidjson::Value(val ? 1 : 0), a);
				}
				};

			// Save wave parameters
			pushIfDiffFloat("waveAmplitude", wb->waveAmplitude);
			pushIfDiffFloat("waveFrequency", wb->waveFrequency);
			pushIfDiffFloat("waveSpeed", wb->waveSpeed);

			// Save water color if different from prefab
			if (!prefabWB || !prefabWB->HasMember("waterColor") ||
				differsFloat((*prefabWB)["waterColor"][0].GetFloat(), wb->waterColor.r) ||
				differsFloat((*prefabWB)["waterColor"][1].GetFloat(), wb->waterColor.g) ||
				differsFloat((*prefabWB)["waterColor"][2].GetFloat(), wb->waterColor.b) ||
				differsFloat((*prefabWB)["waterColor"][3].GetFloat(), wb->waterColor.a)) {

				rapidjson::Value colorArray(rapidjson::kArrayType);
				colorArray.PushBack(wb->waterColor.r, a);
				colorArray.PushBack(wb->waterColor.g, a);
				colorArray.PushBack(wb->waterColor.b, a);
				colorArray.PushBack(wb->waterColor.a, a);
				jw.AddMember("waterColor", colorArray, a);
			}

			pushIfDiffFloat("waterAlpha", wb->waterAlpha);
			pushIfDiffInt("subdivisions", wb->subdivisions);
			pushIfDiffFloat("width", wb->width);
			pushIfDiffFloat("height", wb->height);
			pushIfDiffBool("isTrigger", wb->isTrigger);
			pushIfDiffFloat("buoyancyForce", wb->buoyancyForce);
			pushIfDiffFloat("waterDrag", wb->waterDrag);
			pushIfDiffBool("renderWaves", wb->renderWaves);
			pushIfDiffBool("useTexture", wb->useTexture);

			if (!jw.ObjectEmpty()) {
				comps.AddMember("WaterBody", jw, a);
			}
		}

		// ---- WeaponComponent ----
		if (auto wc = obj->getComponent<WeaponComponent>()) {
			rapidjson::Value jweap(rapidjson::kObjectType);

			// barrelOffset
			rapidjson::Value barrelArr(rapidjson::kArrayType);
			barrelArr.PushBack(wc->barrelOffset.x, a);
			barrelArr.PushBack(wc->barrelOffset.y, a);
			jweap.AddMember("barrelOffset", barrelArr, a);

			// aimAtMouse
			jweap.AddMember("aimAtMouse", wc->aimAtMouse ? 1 : 0, a);

			if (!jweap.ObjectEmpty()) {
				comps.AddMember("WeaponComponent", jweap, a);
			}
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

\param path Scene file path.
*/

void GameObjectManager::loadScene(const std::string& path) {

	// PLEASE STOP CRASHING
#ifdef _DEBUG
	Editor::objSelectionState.selectedObject = nullptr;
	Editor::objSelectionState.selectedPrefab = nullptr;
	Editor::objSelectionState.draggedObject = nullptr;
	Editor::objSelectionState.selectedIndex = -1;
	UndoRedoManager::Instance().clear();

	// do not set current scene to the temp scene (during simulation)
	std::string filename = std::filesystem::path(path).filename().string();
	if (filename != Editor::sceneState.tempSceneName)
		Editor::sceneState.currentSceneName = filename;
#endif

	if (ends_with(path, ".json")) loadSceneFromJson(path);
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

\param path Output scene path.
\param isNew If true, writes an empty or simplified scene header.
*/

void GameObjectManager::saveScene(const std::string& path, bool isNew) const {
	if (ends_with(path, ".json")) saveSceneToJson(path, isNew);
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
		//initialize animation texture
		if (obj->hasComponent<Animation>()) {
			Animation* animation = obj->getComponent<Animation>();
			if (!animation->animState.empty())
			{
				for (AnimateState& as : animation->animState) {
					if (!as.texFile.empty()) {
						TextureData textureData = ResourceManager::getInstance().getTexture(as.texFile);
						as.texHDL = textureData.id;
					}
				}
			}
		}
	}
}


void GameObjectManager::createBulletPool(const std::string& templateBulletName, int poolSize) {
	GameObject* templateBullet = getGameObject(templateBulletName);
	if (!templateBullet) {
		std::cerr << "Error: Template bullet '" << templateBulletName << "' not found!\n";
		return;
	}

	// ensure only 11 bullets (1+10 clone)
	if (templateBullet->getObjectName() != "bullet") renameGameObject(templateBullet, "bullet");

	if (CollisionInfo* c = templateBullet->getComponent<CollisionInfo>()) {
		c->collisionTag = "bullet";
		c->ignoreCollisionsWith.insert("player");
		//c->ignoreCollisionsWith.insert("bullet");
	}
	std::cout << "Creating bullet pool with " << poolSize << " bullets...\n";

	for (int i = 1; i <= poolSize; ++i) {
		std::string bulletName = templateBulletName + std::to_string(i);

		if (getGameObject(bulletName)) {
			std::cout << "  Bullet already exists: " << bulletName << "\n";
			continue;
		}

		GameObject* clonedBullet = CloneGameObject(templateBulletName, bulletName);
		if (!clonedBullet) continue;

		clonedBullet->active = false;

		if (Transform* t = clonedBullet->getComponent<Transform>()) {
			t->x = -1000.0f;
			t->y = -1000.0f;
			t->z = 0.0f;
		}
		templateBullet->active = false; //deactivate the template bullet as well

		if (Physics* p = clonedBullet->getComponent<Physics>()) {
			p->alive = false;
			p->dynamics.velocity = Vector3D{ 0.0f, 0.0f, 0.0f };
		}

		if (CollisionInfo* c = clonedBullet->getComponent<CollisionInfo>()) {
			c->collisionTag = "bullet";
			c->ignoreCollisionsWith.insert("player");
			//c->ignoreCollisionsWith.insert("bullet");
		}
		/*GameObject* clonedBullet = *///CloneGameObject(templateBulletName, bulletName);
		//incase of bullets not cloned properly
		//if (clonedBullet) {
		//	if (clonedBullet->hasComponent<Transform>()) {
		//		Transform* t = clonedBullet->getComponent<Transform>();
		//		/*t->x = -1000.0f;
		//		t->y = -1000.0f;*/
		//	}
		//	if (clonedBullet->hasComponent<Physics>()) {
		//		Physics* p = clonedBullet->getComponent<Physics>();
		//		/*p->alive = false;
		//		p->dynamics.position.x = -1000.0f;
		//		p->dynamics.position.y = -1000.0f;
		//		p->dynamics.velocity.x = 0.0f;
		//		p->dynamics.velocity.y = 0.0f;
		//		p->dynamics.velocity.z = 0.0f;*/
		//	}
		// if (clonedBullet) {
		/*		if (CollisionInfo* c = clonedBullet->getComponent<CollisionInfo>()) {
					c->collisionTag = "bullet";
					c->ignoreCollisionsWith.insert("player");
					c->ignoreCollisionsWith.insert("bullet");
			}
		}*/
		//	std::cout << "  Created: " << bulletName << "\n";
		//}
	}
	std::cout << "Bullet pool creation completed.\n";
}

void GameObjectManager::initializeSimulationResources() {
	// Initialize 
	// for all objects
	for (auto& pair : m_gameObjects) {
		GameObject* obj = pair.second.get();
		if (obj->hasComponent<AudioComponent>()) {
			AudioComponent* audio = obj->getComponent<AudioComponent>();

			// Pre-load all sounds
			for (auto& [channelName, channel] : audio->audioChannels) {
				if (!channel.audioFile.empty()) {
					FMOD::Sound* sound = ResourceManager::getInstance().getSound(channel.audioFile);
					if (!sound) {
						std::cerr << "[GameObjectManager] Failed to preload audio: "
							<< channel.audioFile << std::endl;
					}
				}
			}
		}
	}
}

void GameObjectManager::cleanupSimulationResources() {
	// Stop all playing audio
	for (auto& pair : m_gameObjects) {
		GameObject* obj = pair.second.get();
		if (obj->hasComponent<AudioComponent>()) {
			AudioComponent* audio = obj->getComponent<AudioComponent>();


			for (auto& [channelName, channel] : audio->audioChannels) {
				if (channel.channel) {
					//channel.state = AudioState::Stopped;
					channel.isPendingStop = true;
					channel.channel = nullptr;
				}
			}
		}
	}
}

//clears all game objects from the scene
void GameObjectManager::clearScene() {
	m_gameObjects.clear();     //deletes all GameObjects
	//m_layerManager.clear(); 
}
