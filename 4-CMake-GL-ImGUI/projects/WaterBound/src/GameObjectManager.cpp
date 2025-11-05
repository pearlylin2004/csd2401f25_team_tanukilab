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
#include "JsonIO.h"

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

	// Store the unique_ptr in the map
	m_gameObjects[name] = std::move(newObject);

	return ptr;
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
	//loadFromFile("shaders/Scene/temp_serialize.txt");
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
				renderptr->texChanged = true;
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

void GameObjectManager::loadScene(const std::string& path) {
	if (ends_with(path, ".json")) loadFromJson(path);
	else                          loadFromFile(path);   // keep legacy .txt scenes
}

void GameObjectManager::saveScene(const std::string& path, bool isNew) const {
	if (ends_with(path, ".json")) saveToJson(path, isNew);
	else                          saveToFile(path);
}
