#pragma once
#include "GameObject.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

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
	void getAllGameObjects(std::vector<GameObject*> gameObjects);

private:
	//using unordered map quite a lot huh
	std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameObjects;
};