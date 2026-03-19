/* Start Header ************************************************************************/
/*!
\file       layerManager.h
\author     Hugo Low Ren Hao, low.h, 2402272, 60%
\author 	Varick Teo, v.teo, 2403417, 40%
\par        low.h@digipen.edu
\par        v.teo@digipen.edu
\date       November, 1, 2025
\brief      This file contains the declaration of the LayerManager class.
			It manages multiple layers, assigns GameObjects to them, and provides
			layer-wide visibility and update control for system integration.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
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

	//initialize default layers if none exist
	void initDefaultLayers();

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

	// Removes all layer structures (not just objects)
	void clearAllLayerDefinitions();

	//layer enable/disable by ID
	void setLayerEnabled(int layerID, bool enabled);
	bool isLayerEnabled(int layerID) const;

	//layer enable/disable by name
	void setLayerEnabledByName(const std::string& name, bool enabled);
	bool isLayerEnabledByName(const std::string& name) const;

	//visibility control
	void setLayerVisible(int layerID, bool visible);
	bool isLayerVisible(int layerID) const;

	//update control
	void setLayerUpdatable(int layerID, bool updatable);
	bool isLayerUpdatable(int layerID) const;

	//get objects only from enabled/visible/updatable layers
	std::vector<GameObject*> getVisibleObjects() const;
	std::vector<GameObject*> getUpdatableObjects() const;

	//rename a layer (updates both the layer and the name-to-ID map)
	bool renameLayer(int layerID, const std::string& newName);
	
	//get layer count
	size_t getLayerCount() const;

private:
	//map of layerID to Layer object
	std::map<int, std::unique_ptr<Layer>> m_layers;

	//map of layer name to layerID for lookup
	std::map<std::string, int> m_layerNameToID;
};