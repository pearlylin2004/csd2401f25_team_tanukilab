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
		std::cout << "file not found \n";
	std::cout << "file opened\n";
	std::string checker{};
	std::string objectName;
	std::string shape;
	while (!is.eof())
	{
		for (; is >> checker, checker == "object";);
		if (checker == "object")
		{
			is >> objectName;
			std::unique_ptr<GameObject> newObject = std::make_unique<GameObject>(objectName);
			m_gameObjects.emplace(objectName, std::move(newObject));
		}
		for (; is >> checker, checker == "renderer";);
		if (checker == "renderer")
		{
			Render* renderptr = m_gameObjects[objectName]->addComponent<Render>();
			is >> shape;
			if (shape == "square")
			{
				renderptr->modelRef = renderer::models[static_cast<int>(shape::square)];
			}
		}
		for (; is >> checker, checker == "transform";);
		if (checker == "transform")
		{
			Transform* transformptr = m_gameObjects[objectName]->addComponent<Transform>();
			is >> transformptr->x >> transformptr->y >> transformptr->z;
			is >> transformptr->rotation;
			is >> transformptr->scaleX >> transformptr->scaleY >> transformptr->scaleZ;
		}
	}
}

