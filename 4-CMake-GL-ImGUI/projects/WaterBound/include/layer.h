#pragma once

#include <string>
#include <vector>
#include <unordered_set>

//forward declaration
class GameObject;

class Layer {
public:
	//default constructor
	Layer(const std::string& name, const int& layerID);

	//need to add and remove game objects from layers
	void addGameObject(GameObject* obj);
	bool removeGameObject(GameObject* obj);

	//need to check if object is in layer
	bool hasObject(GameObject* obj) const;

	//get all objects in layer to render/check collisions/etc
	const std::vector<GameObject*>& getObjects() const;

	//get layer id and name
	int getLayerID() const;
	const std::string& getLayerName() const;
	
	//clear all objects from layer
	void clear();

	//get number of objects in layer
	size_t getObjectCount() const;

private:
	std::string m_layerName;
	int m_layerID;
	std::vector<GameObject*> m_objects;
	std::unordered_set<GameObject*> m_objectSet; //for quick lookup
};