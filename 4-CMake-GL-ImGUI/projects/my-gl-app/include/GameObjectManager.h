/* Start Header ************************************************************************/
/*!
\file        GameObjectManager.h
\author      Seow Sin Le, s.sinle, 2401084, 40%
\par         s.sinle@digipen.edu
\date        October, 1st, 2025
\brief       manages all game objects and initializes them with their corresponding components

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "GameObject.h"

//GameObjectManager should hold every existing GameObject in a smart pointer
//so when this manager is destroyed, 
//all other GameObjects are destroyed with it
//no more news and deletes! yippee
class GameObjectManager {
public:
	//create objects
	GameObject* createGameObject(const std::string& name);

	//use this when objects need to interact with each other
	GameObject* getGameObject(const std::string& name);

	//used for all the component systems
	void getAllGameObjects(std::vector<GameObject*>& gameObjects);

	void init();

private:
	//using unordered map quite a lot huh
	std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameObjects;
};