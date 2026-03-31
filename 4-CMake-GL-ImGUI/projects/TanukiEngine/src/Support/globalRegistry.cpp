/* Start Header ************************************************************************/
/*!
\file       globalRegistry.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       January, 24th, 2026
\brief		Handles initialization, validation, and persistence of global
			collision tags using JSON serialization. Provides centralized,
			engine-wide access to shared configuration data.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "globalRegistry.h"

GlobalRegistry& GlobalRegistry::Instance() {
    static GlobalRegistry instance;
    return instance;
}

void GlobalRegistry::init() {
    std::string err;
	if (!JsonIO::LoadCollisionTags(m_basePath + globalFilepath, collisionTags, &err)) {
		std::cerr << "[GlobalRegistry] Failed to load collision tags: " << err << std::endl;
        collisionTags = { "default" };
	}

	collisionTagLookup.clear();
	for (const std::string& tag : collisionTags) {
		collisionTagLookup.insert(tag);
	}
}

void GlobalRegistry::shutdown() {
    collisionTags.clear();
    collisionTagLookup.clear();
}

const std::vector<std::string>& GlobalRegistry::getCollisionTags() const {
	return collisionTags;
}

bool GlobalRegistry::isValidCollisionTag(const std::string& tag) const {
	return collisionTagLookup.find(tag) != collisionTagLookup.end();
}

bool GlobalRegistry::addCollisionTag(const std::string& tag) {
	// check if tag exists
	if (isValidCollisionTag(tag)) {
		return false;
	}

	// check if tag is empty
	if (tag.empty()) {
		return false;
	}

	collisionTags.push_back(tag);
	collisionTagLookup.insert(tag);

	// serialize
	return saveCollisionTag();
}

bool GlobalRegistry::loadCollisionTag() {
	std::string err;
	if (!JsonIO::LoadCollisionTags(m_basePath + globalFilepath, collisionTags, &err)) {
		std::cerr << "[GlobalRegistry] Failed to load collision tags: " << err << std::endl;
		return false;
	}

	collisionTagLookup.clear();
	for (const std::string& tag : collisionTags) {
		collisionTagLookup.insert(tag);
	}

	return true;
}

bool GlobalRegistry::saveCollisionTag() {
	std::string err;
	if (!JsonIO::SaveCollisionTags(m_basePath + globalFilepath, collisionTags, &err)) {
		std::cerr << "[GlobalRegistry] Failed to save collision tags: " << err << std::endl;
		return false;
	}

	return true;
}