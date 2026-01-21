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
		if (node.imp == NodeImplementation::ON_START) {
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::update(GameObject* obj, VisualLC* logic, float dt) {
	(void)dt; // to use in the future
	if (!obj || !logic) return;

	// find and execute all ON UPDATE nodes
	for (LogicNode& node : logic->nodes) {
		if (node.imp == NodeImplementation::ON_UPDATE) {
			executeNode(obj, logic, node.id);
		}
	}
}

void VisualLogicSystem::onCollisionEnter(GameObject* obj, VisualLC* logic, GameObject* other) {
	if (!obj || !logic || !other) return;

	// execute ON COLLISION ENTER nodes
	for (LogicNode& node : logic->nodes) {
		if (node.imp == NodeImplementation::ON_COLLISION) {
			logic->stringVar["_collidedWith"] = other->getObjectName();
			executeNode(obj, logic, node.id);
		}
	}
}

//void VisualLogicSystem::onCollisionExit(GameObject* obj, VisualLC* logic, GameObject* other) {
//	if (!obj || !logic || !other) return;
//
//	// execute ON COLLISION EXIT nodes
//	for (auto& node : logic->nodes) {
//		if (node.imp == NodeImplementation::ON_COLLISION) {
//			logic->stringVariables["_collidedWith"] = other->getObjectName();
//			executeNode(obj, logic, node.id);
//		}
//	}
//}

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
			std::string pin = result ? "True" : "False";
			int nextNodeId = findNextNode(logic, node->id, pin);
			if (nextNodeId != -1) {
				executeNode(obj, logic, nextNodeId);
			}
			return;
		}
	}

	// execute next connected node (NOT for condition)
	int nextNodeId = findNextNode(logic, node->id, "Out");
	if (nextNodeId != -1) {
		executeNode(obj, logic, nextNodeId);
	}
}

void VisualLogicSystem::executeEventNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	(void)obj; // to use in future
	(void)node; // to use in future

	logic->isExecuting = true;
	logic->currentExecutingNodeId = node.id;
}

void VisualLogicSystem::executeActionNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	(void)logic; // to use in future

	// print message
	if (node.imp == NodeImplementation::PRINT_MESSAGE) {
		std::string msg = node.parameters.count("message") ? node.parameters["message"] : "I love GAM";

		DebugLog::addMessage(msg, DebugMode::PlaySimul);
		std::cout << msg << std::endl;
	}
	else if (node.imp == NodeImplementation::MOVE_OBJECT) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.f;
			float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.f;
			
			transform->x += x;
			transform->y += y;

			DebugLog::addMessage("Moved object by (" +
				std::to_string(x) + ", " + std::to_string(y) + ")\n", DebugMode::PlaySimul);
		}
	}
	else if (node.imp == NodeImplementation::ROTATE_OBJECT) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;

			transform->rotation += angle;

			DebugLog::addMessage("Rotated object by " +
				std::to_string(angle) + " degrees\n", DebugMode::PlaySimul);
		}
	}
	else if (node.imp == NodeImplementation::SET_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		logic->floatVar[varName] = value;

		DebugLog::addMessage("Set variable '" + varName + "' to " +
			std::to_string(value) + "\n", DebugMode::PlaySimul);
	}
}

bool VisualLogicSystem::executeConditionNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	// check key pressed/mouse clicked/compare numbers
	(void)obj; // to use in future
	(void)logic; // to use in future

	if (node.imp == NodeImplementation::CHECK_KEY_HELD) {
		std::string keyStr = node.parameters.count("key") ?
			node.parameters["key"] : "W";

		int key = JsonIO::stringToGLFWKey(keyStr);

		if (key != GLFW_KEY_UNKNOWN)
			return InputHandler::isKeyHeld(key);

		return false;
	}
	else if (node.imp == NodeImplementation::COMPARE_VALUE) {
		std::string property = node.parameters.count("property") ?
			node.parameters["property"] : "rotation";

		std::string comparison = node.parameters.count("comparison") ?
			node.parameters["comparison"] : "greater";

		float threshold = node.parameters.count("threshold") ?
			std::stof(node.parameters["threshold"]) : 0.f;

		float currentValue = 0.f;

		Transform* transform = obj->getComponent<Transform>();
		Physics* physics = obj->getComponent<Physics>();

		if (property == "rotation" && transform) {
			currentValue = transform->rotation;
		}
		else if (property == "x" && transform) {
			currentValue = transform->x;
		}
		else if (property == "y" && transform) {
			currentValue = transform->y;
		}
		else if (property == "z" && transform) {
			currentValue = transform->z;
		}
		else if (property == "scaleX" && transform) {
			currentValue = transform->scaleX;
		}
		else if (property == "scaleY" && transform) {
			currentValue = transform->scaleY;
		}
		else if (property == "scaleZ" && transform) {
			currentValue = transform->scaleZ;
		}
		else if (property == "velX" && physics) {
			currentValue = physics->velX;
		}
		else if (property == "velY" && physics) {
			currentValue = physics->velY;
		}
		else {
			// cannot find
			return false;
		}

		// compare
		if (comparison == "greater") {
			return currentValue > threshold;
		}
		else if (comparison == "less") {
			return currentValue < threshold;
		}
		else if (comparison == "equal") {
			// epsilon for float comparison
			float epsilon = 0.1f;
			return std::abs(currentValue - threshold) < epsilon;
		}

		return false;
	}
	else if (node.imp == NodeImplementation::COMPARE_VAR) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		std::string comparison = node.parameters.count("comparison") ?
			node.parameters["comparison"] : "greater";

		float threshold = node.parameters.count("threshold") ?
			std::stof(node.parameters["threshold"]) : 0.f;

		// get variable value
		float currentValue = 0.f;
		if (logic->floatVar.count(varName)) {
			currentValue = logic->floatVar[varName];
		}

		// compare
		if (comparison == "greater") {
			return currentValue > threshold;
		}
		else if (comparison == "less") {
			return currentValue < threshold;
		}
		else if (comparison == "equal") {
			float epsilon = 0.1f;
			return std::abs(currentValue - threshold) < epsilon;
		}

		return false;
	}

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