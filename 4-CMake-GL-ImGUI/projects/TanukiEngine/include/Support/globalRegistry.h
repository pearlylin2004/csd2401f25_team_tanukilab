/* Start Header ************************************************************************/
/*!
\file       globalRegistry.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       January, 24th, 2026
\brief      Manages global collision tags, including validation, add/remove,
			and persistence through JSON load/save. Implemented as a singleton
			to provide centralized access across the game.
			Customize the collsion tags by editing global_settings.json

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include "JsonIO.h"

class GlobalRegistry {
public:
	// Access the singleton instance of GlobalRegistry
	static GlobalRegistry& Instance();

	// Initialize the registry (load defaults or settings from file)
	void init();
	void shutdown();

	// Get the list of all registered collision tags
	const std::vector<std::string>& getCollisionTags() const;
	// Check if a collision tag is valid / exists
	bool isValidCollisionTag(const std::string& tag) const;
	// Add a new collision tag if it does not already exist
	// Returns true if added successfully
	bool addCollisionTag(const std::string& tag);

	// Load collision tags from persistent storage
	bool loadCollisionTag();

	// Save collision tags to persistent storage
	bool saveCollisionTag();

	// Set the base path for storing global settings
	void setBasePath(const std::string& path) { m_basePath = path; }


private:
	GlobalRegistry() = default;
	~GlobalRegistry() = default;

	// Delete copy constructor and assignment to enforce singleton pattern
	GlobalRegistry(const GlobalRegistry&) = delete;
	GlobalRegistry& operator=(const GlobalRegistry&) = delete;

	std::vector<std::string> collisionTags;
	std::unordered_set<std::string> collisionTagLookup;
	std::string m_basePath;
	std::string globalFilepath = "/resources/global_setting.json";
};