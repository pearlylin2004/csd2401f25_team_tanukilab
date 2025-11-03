#pragma once

#include <memory>
#include <map>

#include "layer.h"

class GameObject;
class GameObjectManager;

class LayerManager {
public:
	//default constructor
	LayerManager();

	//create a new layer
	Layer* createLayer(const std::string& name, int layerID);

	//get layer by id
	Layer* getLayer(int layerID);

	//get layer by name
	Layer* getLayerByName(const std::string& name);

	//assign object to layer
	bool assignObjectToLayer(GameObject* obj, int layerID);

	//remove object from layer
	void removeObjectFromLayer(GameObject* obj);

	//get object's layer
	int getObjectLayer(GameObject* obj) const;

	//get all layers
	std::vector<Layer*> getAllLayers() const;

	//clear all objects from all layers
	void clearAllLayers();


private:
	//map of layerID to Layer object
	std::map<int, std::unique_ptr<Layer>> m_layers;

	//map of layer name to layerID for lookup
	std::map<std::string, int> m_layerNameToID;
};