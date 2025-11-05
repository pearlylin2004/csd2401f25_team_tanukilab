/* Start Header ************************************************************************/
/*!
\file       input.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 15th, 2025
\brief      A static utility class that manages all input handling for a GLFW window.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <unordered_map>
#include <string>
#include "GameObjectManager.h"

//struct PrefabData {
//	std::string name;
//	std::string id;
//};

/*
HOW TO USE THIS
- PrefabManager::Instance() -> to access the singleton instance
e.g. PrefabManager::Instance().instantiate("player1"); -> to instantiate a prefab with id "player1"
*/

class PrefabManager {
public:
	// singleton access
	static PrefabManager& Instance();

	void loadPrefabRegistry(const std::string& filename);

	std::string& getPrefabPath(const std::string& prefabID);
	const std::string& getPrefabPath(const std::string& prefabID) const;

	std::string& getPrefabName(const std::string& prefabID);
	const std::string& getPrefabName(const std::string& prefabID) const;

	const rapidjson::Document* getPrefabJson(const std::string& prefabID) const;

	const rapidjson::Value* getPrefabJsonComp(const rapidjson::Document* prefabDoc, const std::string& compName) const;

	//GameObject* instantiate(const std::string& id, GameObjectManager& manager);
	GameObject* instantiate(GameObject* go, GameObjectManager& manager);

private:
	struct PrefabInfo {
		std::string name;
		std::string path;
	};

	// prefab registry mapping prefab ID to PrefabInfo
	// format {prefabID: {name, pathsure}}
	std::unordered_map<std::string, PrefabInfo> m_prefabRegistry;

	// prefab jsons cache aft first load
	std::unordered_map<std::string, rapidjson::Document> m_prefabCache;
};