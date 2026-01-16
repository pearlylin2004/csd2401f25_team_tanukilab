/* Start Header ************************************************************************/
/*!
\file       visualLogic.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       January, 16th, 2026
\brief      T

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"

void VisualLogicSystem::initAll(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();
			init(obj, logic);
		}
	}
}

void VisualLogicSystem::updateAll(GameObjectManager& manager, float dt) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();
			update(obj, logic, dt);
		}
	}
}

void VisualLogicSystem::init(GameObject* obj, VisualLC* logic) {
	if (!obj || !logic) return;

	// find and execute all ON START nodes
	for (LogicNode& node : logic->nodes) {
		if (node.name == "On Start") {
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::update(GameObject* obj, VisualLC* logic, float dt) {
	if (!obj || !logic) return;

	// find and execute all ON UPDATE nodes
	for (LogicNode& node : logic->nodes) {
		if (node.name == "On Update") {
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::onCollisionEnter(GameObject* obj, VisualLC* logic, GameObject* other) {
	if (!obj || !logic || !other) return;

	// execute ON COLLISION ENTER nodes
	for (LogicNode& node : logic->nodes) {
		if (node.name == "On Collision") {
			logic->stringVariables["_collidedWith"] = other->getObjectName();
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::onCollisionExit(GameObject* obj, VisualLC* logic, GameObject* other) {
	if (!obj || !logic || !other) return;

	// execute ON COLLISION EXIT nodes
	for (auto& node : logic->nodes) {
		if (node.name == "On Collision Exit") {
			logic->stringVariables["_collidedWith"] = other->getObjectName();
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::executeNode(GameObject* obj, VisualLC* logic, int nodeId) {
	if (!obj || !logic) return;

	LogicNode* node = findNode(logic, nodeId);
	if (!node) return;

	// execute based on node type
	switch (node->type) {
		case NodeType::EVENT:
			executeEventNode(obj, logic, *node);
			break;
		case NodeType::ACTION:
			executeActionNode(obj, logic, *node);
			break;
		case NodeType::CONDITION: {
			bool result = executeConditionNode(obj, logic, *node);

			// find next node based on TRUE/FALSE
			std::string pin = result ? "TRUE" : "FALSE";
			int nextNodeId = findNextNode(logic, node->id, pin);
			if (nextNodeId != -1) {
				executeNode(obj, logic, nextNodeId);
			}
			return;
		}
	}

	// execute next connected node (NOT for condition)
	int nextNodeId = findNextNode(logic, node->id, "OUT");
	if (nextNodeId != -1) {
		executeNode(obj, logic, nextNodeId);
	}
}

void VisualLogicSystem::executeEventNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	logic->isExecuting = true;
	logic->currentExecutingNodeId = node.id;

	DebugLog::addMessage("Executing Event Node: " + node.name + " (ID: " + std::to_string(node.id) + ")", DebugMode::PlaySimul);
}

void VisualLogicSystem::executeActionNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	// print message
	if (node.name == "Print Message") {
		std::string msg = node.parameters.count("message") ? node.parameters["message"] : "I love GAM";

		DebugLog::addMessage("Visual Logic Print: " + msg, DebugMode::PlaySimul);
		std::cout << "[Visual Logic Print]: " << msg << std::endl;
	}
	else if (node.name == "Move Object") {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.0f;
			float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.0f;
			
			transform->x += x;
			transform->y += y;

			DebugLog::addMessage("Moved object by (" +
				std::to_string(x) + ", " + std::to_string(y) + ")\n");
		}
	}
}

bool VisualLogicSystem::executeConditionNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	// check key pressed/mouse clicked/compare numbers

	return true;
}

int VisualLogicSystem::findNextNode(VisualLC* logic, int fromNodeId, const std::string& outputPin) {
	for (const LogicConnection& conn : logic->connections) {
		if (conn.fromNodeId == fromNodeId && conn.outputPin == outputPin) {
			return conn.toNodeId;
		}
	}

	// no connection found
	return -1;
}

LogicNode* VisualLogicSystem::findNode(VisualLC* logic, int nodeId) {
	for (LogicNode& node : logic->nodes) {
		if (node.id == nodeId) {
			return &node;
		}
	}

	return nullptr;
}