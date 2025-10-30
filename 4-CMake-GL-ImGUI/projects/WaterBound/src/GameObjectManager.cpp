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
	loadFromFile("shaders/temp_serialize.txt");

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
				is >> renderptr->hasAnimation;
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

		m_gameObjects["player"]->addComponent<Input>(1.f);

	}
}

void GameObjectManager::saveToFile(const std::string& filename) {
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

		os << "\n";
	}

	std::cout << "Scene saved to " << filename << "\n";
}