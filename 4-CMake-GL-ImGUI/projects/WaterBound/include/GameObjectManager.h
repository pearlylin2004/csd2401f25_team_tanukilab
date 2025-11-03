/* Start Header ************************************************************************/
/*!
\file        GameObjectManager.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 80%
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

#pragma once
#include <fstream>
#include <sstream>
#include <iomanip>

#include "GameObject.h"
#include "layerManager.h"

//GameObjectManager should hold every existing GameObject in a smart pointer
//so when this manager is destroyed, 
//all other GameObjects are destroyed with it
//no more news and deletes! yippee

// This class is responsible for creating, storing, and providing access to game objects.
class GameObjectManager {
public:
	//create objects
	GameObject* createGameObject(const std::string& name);

	//use this when objects need to interact with each other
	GameObject* getGameObject(const std::string& name);

	// delete objects
	void deleteGameObject(GameObject* object);

	//clone an existing object
	GameObject* CloneGameObject(const std::string& sourceName, const std::string& newName);

	//used for all the component systems
	void getAllGameObjects(std::vector<GameObject*>& gameObjects);

	//initialize game objects from a file
	void init();

	bool renameGameObject(GameObject* obj, const std::string& newName);

	// load from file (read)
	void loadFromFile(const std::string& filename);

	// save to file (write)
	void saveToFile(const std::string& filename) const;

	void loadFromJson(const std::string& filename);

	void saveToJson(const std::string& filename, bool isNew) const;

	void loadScene(const std::string& path);

	void saveScene(const std::string& path, bool isNew = false) const;

	//layering manager stuff
	LayerManager& getLayerManager();

	bool assignObjectToLayer(GameObject* obj, int layerID);

	std::vector<GameObject*> getObjectsInLayer(int layerID);

private:
	//map of all game objects
	std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameObjects;

	LayerManager m_layerManager;
};