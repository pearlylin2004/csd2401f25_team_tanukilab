/* Start Header ************************************************************************/
/*!
\file       globalRegistry.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       January, 24th, 2026
\brief      Manages global collision tags, including validation, add/remove,
			and persistence through JSON load/save. Implemented as a singleton
			to provide centralized access across the engine. 
			Will implement more global settings in the future.

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
	static GlobalRegistry& Instance();

	void init();
	void shutdown();

	// collision tags
	const std::vector<std::string>& getCollisionTags() const;
	bool isValidCollisionTag(const std::string& tag) const;
	bool addCollisionTag(const std::string& tag);

	bool loadCollisionTag();
	bool saveCollisionTag();

private:
	GlobalRegistry() = default;
	~GlobalRegistry() = default;

	GlobalRegistry(const GlobalRegistry&) = delete;
	GlobalRegistry& operator=(const GlobalRegistry&) = delete;

	std::vector<std::string> collisionTags;
	std::unordered_set<std::string> collisionTagLookup;
	std::string globalFilepath = "/resources/global_setting.json";
};