/* Start Header ************************************************************************/
/*!
\file       layer.cpp
\author     Hugo Low Ren Hao, low.h, 2402272

\par        low.h@digipen.edu
\date       November, 1, 2025
\brief      This file contains the Implementation of the Layer class.
			It manages a collection of GameObjects assigned to a specific layer.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "layer.h"
#include "GameObject.h"
#include <algorithm>

Layer::Layer(const std::string& name, const int& layerID) 
	: m_layerName(name)
	, m_layerID(layerID)
	, m_enabled(true)
	, m_visible(true)
	, m_updatable(true) 
{}

void Layer::addGameObject(GameObject* obj) {
	if (!obj) {
		std::cerr << "Layer Manager Error: Attempted to add a null GameObject to the layer.\n";
		return;
	}

	if(m_objectSet.find(obj) == m_objectSet.end()) {
		m_objects.push_back(obj);
		m_objectSet.insert(obj);
	}
}

bool Layer::removeGameObject(GameObject* obj) {
	if (!obj) {
		std::cerr << "Layer Manager Error: Attempted to remove a null GameObject from the layer.\n";
		return false;
	}

	auto setIt = m_objectSet.find(obj);
	if (setIt == m_objectSet.end()) {
		return false;
	}

	m_objectSet.erase(setIt);

	auto vecIt = std::find(m_objects.begin(), m_objects.end(), obj);
	if(vecIt != m_objects.end()) {
		m_objects.erase(vecIt);
		std::cout << "Layer Manager: GameObject removed from layer successfully.\n";
		return true;
	}

	return false;
}

bool Layer::hasObject(GameObject* obj) const {
	return m_objectSet.find(obj) != m_objectSet.end();
}

const std::vector<GameObject*>& Layer::getObjects() const {
	return m_objects;
}

int Layer::getLayerID() const {
	return m_layerID;
}

const std::string& Layer::getLayerName() const {
	return m_layerName;
}

void Layer::clear() {
	m_objects.clear();
	m_objectSet.clear();
}

size_t Layer::getObjectCount() const {
	return m_objects.size();
}

//master enable/disable - affects both visibility and updates
void Layer::setEnabled(bool enabled) {
	m_enabled = enabled;
	m_visible = enabled;
	m_updatable = enabled;
}

bool Layer::isEnabled() const {
	return m_enabled;
}

//visibility only - for rendering
void Layer::setVisible(bool visible) {
	m_visible = visible;
}

bool Layer::isVisible() const {
	return m_enabled && m_visible;
}

//updatable only - for physics/logic
void Layer::setUpdatable(bool updatable) {
	m_updatable = updatable;
}

bool Layer::isUpdatable() const {
	return m_enabled && m_updatable;
}