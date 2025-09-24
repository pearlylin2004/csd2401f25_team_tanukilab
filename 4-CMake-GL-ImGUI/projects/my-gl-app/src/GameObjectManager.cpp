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
void GameObjectManager::getAllGameObjects(std::vector<GameObject*> gameObjects) {
	//clear vector just in case
	gameObjects.clear();

	gameObjects.reserve(m_gameObjects.size());

	for (const auto& iterator : m_gameObjects) {
		gameObjects.push_back(iterator.second.get());
	}
}

