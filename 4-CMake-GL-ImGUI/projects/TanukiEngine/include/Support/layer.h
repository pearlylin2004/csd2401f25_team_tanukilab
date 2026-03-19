/* Start Header ************************************************************************/
/*!
\file       layer.h
\author     Hugo Low Ren Hao, low.h, 2402272, 60%
\author 	 Varick Teo, v.teo, 2403417, 40%
\par        low.h@digipen.edu
\par        v.teo@digipen.edu
\date       October, 30, 2025
\brief      This file contains the declaration of the Layer class.
			It manages a collection of GameObjects assigned to a specific layer
			with separate visibility and update control flags.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

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
	
	//set layer name (for editor)
	void setLayerName(const std::string& name);
	
	//clear all objects from layer
	void clear();

	//get number of objects in layer
	size_t getObjectCount() const;

	//layer enable/disable - controls visibility, physics, and logic updates
	void setEnabled(bool enabled);
	bool isEnabled() const;

	//separate flags for finer control (optional)
	void setVisible(bool visible);
	bool isVisible() const;

	void setUpdatable(bool updatable);
	bool isUpdatable() const;

private:
	std::string m_layerName;
	int m_layerID;
	std::vector<GameObject*> m_objects;
	std::unordered_set<GameObject*> m_objectSet; //for quick lookup

	//layer state flags - single flag check per layer (efficient)
	bool m_enabled = true;    //master toggle - affects both visibility and updates
	bool m_visible = true;    //controls rendering only
	bool m_updatable = true;  //controls physics/logic updates only
};