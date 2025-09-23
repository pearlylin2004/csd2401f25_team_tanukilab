#include "Component.cpp"
#include <unordered_map>

class GameObjectManager {
public:
	GameObject* createGameObject(const std::string& name) {

		//creates the thing to be returned
		std::unique_ptr<GameObject> gameObject = std::make_unique<GameObject>();
		GameObject* ptr = gameObject.get();//gets the ptr
		// use std::move() cus unique_ptr cant be copied
		m_gameObjects[name] = std::move(gameObject);
		return ptr;
	}

	GameObject* getGameObject(const std::string& name) {
		//it is iterator btw
		//auto will auto-detect the type of the iterator
		auto it = m_gameObjects.find(name);
		if (it != m_gameObjects.end()) {
			return it->second.get();
		}
		return nullptr; // not found
	}

	void removeGameObject(const std::string& name) {
		m_gameObjects.erase(name);
	}

	void updateAll(float deltaTime) {
		for (auto& pair : m_gameObjects) {
			pair.second->update(deltaTime);
		}
	}

private:
	std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameObjects;
};