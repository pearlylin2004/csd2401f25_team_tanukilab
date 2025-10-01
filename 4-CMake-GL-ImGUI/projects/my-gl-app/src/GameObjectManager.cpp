/* Start Header ************************************************************************/
/*!
\file        GameObjectManager.cpp
\author      Seow Sin Le, s.sinle, 2401084, 40%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 60%
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

//create objects
GameObject* GameObjectManager::createGameObject(const std::string& name) {
	//create object
	auto newObject = std::make_unique<GameObject>(name);
	//get raw pointer to return
	GameObject* ptr = newObject.get();
	//move object into the map
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
	std::ifstream is{ "shaders/temp_serialize.txt" };
	if (!is)
	{
		std::cerr << "file not found: shaders/temp_serialize.txt\n";
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
		if(checker == "collision")
		{
			m_gameObjects[objectName]->addComponent<CollisionInfo>();
			std::cout << "collision component added\n";
		}

		m_gameObjects["player"]->addComponent<Input>(1.f);
		/*m_gameObjects[objectName]->addComponent<Velocity>();*/
		
	}
}

