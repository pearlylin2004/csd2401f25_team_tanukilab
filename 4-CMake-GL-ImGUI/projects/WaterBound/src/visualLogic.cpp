/* Start Header ************************************************************************/
/*!
\file       visualLogic.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       January, 16th, 2026
\brief      This source file implements the Visual Logic System, which manages visual scripting for game			objects. It handles the execution of nodes in a visual logic component (VisualLC) attached			to game objects, following an Event-Condition-Action (ECA) paradigm.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"
#include "JsonIO.h"

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

	logic->hasHoverNodes = false;
	for (LogicNode& node : logic->nodes) {
		// cache whether obj has hover nodes
		if (node.imp == NodeImplementation::ON_HOVER_ENTER ||
			node.imp == NodeImplementation::WHILE_HOVERING ||
			node.imp == NodeImplementation::ON_HOVER_EXIT) {
			logic->hasHoverNodes = true;
			break;
		}

		logic->hoverNodesChecked = true;

		// find and execute all ON START nodes
		if (node.imp == NodeImplementation::ON_START) {
			logic->currentExePath.clear();
			executeNode(obj, logic, node.id, 1 / 60.f);
		}
	}
}

void VisualLogicSystem::update(GameObject* obj, VisualLC* logic, float dt) {
	if (!obj || !logic) return;


	/* -------- Accumulate timer -------- */
	for (auto& [timerName, elapsedTime] : logic->timers) elapsedTime += dt;
	/* -------- END -------- */


	/* -------- Check collisions -------- */
	for (GameObject* collided : logic->curCollision) {
		// ON COLLISION ENTER
		if (logic->preCollision.find(collided) == logic->preCollision.end()) {
			for (LogicNode& node : logic->nodes) {
				if (node.imp == NodeImplementation::ON_COLLISION_ENTER) {
					if (shouldTriggerCollision(node, collided)) {
						logic->stringVar["_collidedWith"] = collided->getObjectName();
						executeNode(obj, logic, node.id, dt);
					}
				}
			}
		}
	}

	for (GameObject* collided : logic->curCollision) {
		// WHILE COLLIDING
		for (LogicNode& node : logic->nodes) {
			if (node.imp == NodeImplementation::WHILE_COLLIDING) {
				if (shouldTriggerCollision(node, collided)) {
					logic->stringVar["_collidedWith"] = collided->getObjectName();
					executeNode(obj, logic, node.id, dt);
				}
			}
		}
	}

	for (GameObject* collided : logic->preCollision) {
		// ON COLLISION EXIT
		if (logic->curCollision.find(collided) == logic->curCollision.end()) {
			for (LogicNode& node : logic->nodes) {
				if (node.imp == NodeImplementation::ON_COLLISION_EXIT) {
					if (shouldTriggerCollision(node, collided)) {
						logic->stringVar["_collidedWith"] = collided->getObjectName();
						executeNode(obj, logic, node.id, dt);
					}
				}
			}
		}
	}

	// update collision status for next frane
	logic->preCollision = logic->curCollision;
	logic->curCollision.clear();

	/* -------- END -------- */


	/* -------- Safety Check jic haven check -------- */
	if (!logic->hoverNodesChecked) {
		// Cache it now if not done yet (safety check)
		logic->hasHoverNodes = false;
		for (const LogicNode& node : logic->nodes) {
			if (node.imp == NodeImplementation::ON_HOVER_ENTER ||
				node.imp == NodeImplementation::WHILE_HOVERING ||
				node.imp == NodeImplementation::ON_HOVER_EXIT) {
				logic->hasHoverNodes = true;
				break;
			}
		}
		logic->hoverNodesChecked = true;
	}
	/* -------- END -------- */


	/* -------- Check hover -------- */
	if (logic->hasHoverNodes) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			Vector2D mouseWorldPos = InputHandler::getWorldMousePosition();

#ifdef _DEBUG
			// if playing in editor then use ImGui viewport mouse pos
			if (UISystem::isShowUI()) {
				ImVec2 scenePos; ImVec2 sceneSize;
				EditorManager::getScenePosNSize(scenePos, sceneSize);

				mouseWorldPos = InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize);
			}
#endif

			float halfW = transform->scaleX / 2.f;
			float halfH = transform->scaleY / 2.f;

			bool isHoveringNow = (mouseWorldPos.x >= transform->x - halfW &&
				mouseWorldPos.x <= transform->x + halfW &&
				mouseWorldPos.y >= transform->y - halfH &&
				mouseWorldPos.y <= transform->y + halfH);

			// ON HOVER ENTER
			if (isHoveringNow && !logic->wasHoveredLastFrame) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::ON_HOVER_ENTER) {
						logic->currentExePath.clear();

						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			// WHILE HOVERING
			if (isHoveringNow) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::WHILE_HOVERING) {
						logic->currentExePath.clear();

						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			// ON HOVER EXIT
			if (!isHoveringNow && logic->wasHoveredLastFrame) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::ON_HOVER_EXIT) {
						logic->currentExePath.clear();

						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			logic->wasHoveredLastFrame = isHoveringNow;
		}
	}
	/* -------- END -------- */

	for (LogicNode& node : logic->nodes) {
		// find and execute all ON UPDATE nodes
		if (node.imp == NodeImplementation::ON_UPDATE) {
			logic->currentExePath.clear();

			executeNode(obj, logic, node.id, dt);
		}
	}
}

void VisualLogicSystem::executeNode(GameObject* obj, VisualLC* logic, int nodeId, float dt) {
	if (!obj || !logic) return;

	if (logic->currentExePath.count(nodeId)) {
		// oops executed node, return to avoid infinite loop
		return;
	}

	logic->currentExePath.insert(nodeId);

	LogicNode* node = findNode(logic, nodeId);
	if (!node) {
		logic->currentExePath.erase(nodeId);
		return;
	}

	// execute based on node type
	switch (node->type) {
		case NodeType::EVENT:
			executeEventNode(obj, logic, *node);
			break;
		case NodeType::ACTION:
			executeActionNode(obj, logic, *node, dt);
			break;
		case NodeType::CONDITION: {
			bool result = executeConditionNode(obj, logic, *node, dt);

			// find next node based on TRUE/FALSE
			std::string pin = result ? "True" : "False";
			std::vector<int> nextNodes = findAllNextNode(logic, node->id, pin);
			for (int nextID : nextNodes) {
				executeNode(obj, logic, nextID, dt);
			}

			logic->currentExePath.erase(nodeId);
			return;
		}
	}

	// execute next connected node (NOT for condition)
	std::vector<int> nextNodes = findAllNextNode(logic, node->id, "Out");
	for (int nextID : nextNodes) {
		executeNode(obj, logic, nextID, dt);
	}

	logic->currentExePath.erase(nodeId);
}

void VisualLogicSystem::executeEventNode(GameObject* obj, VisualLC* logic, LogicNode& node) {
	(void)obj; // to use in future
	(void)node; // to use in future

	logic->isExecuting = true;
	logic->currentExecutingNodeId = node.id;
}

void VisualLogicSystem::executeActionNode(GameObject* obj, VisualLC* logic, LogicNode& node, float dt) {
	//(void)logic; // to use in future

	// print message
	if (node.imp == NodeImplementation::PRINT_MESSAGE) {
		// static int count{};
		std::string msg = node.parameters.count("message") ? node.parameters["message"] : "I love GAM";

		DebugLog::addMessage(msg, DebugMode::PlaySimul);
		std::cout << msg << /*count++ <<*/ std::endl;
	}
	else if (node.imp == NodeImplementation::MOVE_OBJECT) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.f;
			float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.f;
			
			transform->x += x * dt;
			transform->y += y * dt;

			/*DebugLog::addMessage("Moved object by (" +
				std::to_string(x) + ", " + std::to_string(y) + ")\n", DebugMode::PlaySimul);*/
		}
	}
	else if (node.imp == NodeImplementation::ROTATE_OBJECT) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;

			transform->rotation += angle * dt;

			/*DebugLog::addMessage("Rotated object by " +
				std::to_string(angle) + " degrees\n", DebugMode::PlaySimul);*/
		}
	}
	else if (node.imp == NodeImplementation::SET_ROTATION) {
		Transform* transform = obj->getComponent<Transform>();
		if (transform) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;
			transform->rotation = angle;
		}
	}
	else if (node.imp == NodeImplementation::SET_POSITION) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float x = node.parameters.count("x") ? std::stof(node.parameters["x"]) : 0.f;
			float y = node.parameters.count("y") ? std::stof(node.parameters["y"]) : 0.f;

			transform->x = x;
			transform->y = y;
		}
	}
	else if (node.imp == NodeImplementation::SET_SCALE) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float scaleX = node.parameters.count("scaleX") ? std::stof(node.parameters["scaleX"]) : 1.f;
			float scaleY = node.parameters.count("scaleY") ? std::stof(node.parameters["scaleY"]) : 1.f;

			transform->scaleX = scaleX;
			transform->scaleY = scaleY;
		}
	}
	else if (node.imp == NodeImplementation::SCALE_PROPORTIONAL) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			float scale = node.parameters.count("scale") ? std::stof(node.parameters["scale"]) : 2.f;

			transform->scaleX *= scale;
			transform->scaleY *= scale;
		}
	}
	else if (node.imp == NodeImplementation::SET_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		logic->floatVar[varName] = value;
	}
	else if (node.imp == NodeImplementation::PLAY_SOUND) {
		// component/audio checking
		AudioComponent* audio = obj->getComponent<AudioComponent>();
		if (!audio) {
			std::cerr << "[VisualLogic] Object has no AudioComponent!" << std::endl;
			return;
		}
		if (!node.parameters.count("channel")) {
			std::cerr << "[VisualLogic] No channel specified for Play Sound node!" << std::endl;
			return;
		}

		// get channel
		std::string channelName = node.parameters.at("channel");
		if (channelName.empty()) {
			std::cerr << "[VisualLogic] Channel name is empty for Play Sound node!" << std::endl;
			return;
		}

		// find channel
		AudioChannel* channel = audio->getChannel(channelName);
		if (!channel) {
			std::cerr << "[VisualLogic] Channel " << channelName << " not found" << std::endl;
			return;
		}
		if (channel->audioFile.empty()) {
			std::cerr << "[VisualLogic] No audio file specified for channel " << channelName << "!" << std::endl;
			return;
		}

		channel->isPendingPlay = true;
	}
	else if (node.imp == NodeImplementation::SET_STATE) {
		StateMachine* sm = obj->getComponent<StateMachine>();

		if (!sm) {
			std::cerr << "[VisualLogic] Object has no StateMachine component!" << std::endl;
			return;
		}

		std::string stateName = node.parameters.count("state") ? node.parameters["state"] : JsonIO::stateNames[0];
		
		sm->state = JsonIO::parsePlayerState(stateName);
		sm->stateTime = 0.f;
	}
	else if (node.imp == NodeImplementation::RESET_TIMER) {
		// Generate timer name from node's timerGroup parameter
		std::string timerName = node.parameters.count("timerName") ?
			node.parameters["timerName"] : "timer_0";

		// Reset timer to 0
		logic->timers[timerName] = 0.f;

		std::cout << "after reset: " << logic->timers[timerName] << std::endl;
	}
}

bool VisualLogicSystem::executeConditionNode(GameObject* obj, VisualLC* logic, LogicNode& node, float dt) {
	// check key pressed/mouse clicked/compare numbers
	(void)obj; // to use in future
	(void)logic; // to use in future
	(void)dt; // to use in future

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
	else if (node.imp == NodeImplementation::CHECK_STATE) { 
		std::string target = node.parameters.count("state") ? node.parameters["state"] : JsonIO::stateNames[0];
		
		StateMachine* sm = obj->getComponent<StateMachine>();
		if (!sm) return false;

		return JsonIO::parsePlayerState(target) == sm->state;
	}
	else if (node.imp == NodeImplementation::CHECK_TIMER) {
		float threshold = node.parameters.count("threshold") ? std::stof(node.parameters["threshold"]) : 1.f;

		// generate timer name so one obj can have multiple timers
		std::string timerName = node.parameters.count("timerName") ? node.parameters["timerName"] : "timer_0";

		if (!logic->timers.count(timerName)) logic->timers[timerName] = 0.f;

		std::cout << "checking timer " << timerName << ": " << logic->timers[timerName] << " vs " << threshold << std::endl;

		return logic->timers[timerName] > threshold;
	}

	return true;
}

std::vector<int> VisualLogicSystem::findAllNextNode(VisualLC* logic, int fromNodeId, const std::string& outputPin) {
	std::vector<int> nodes;

	for (const LogicConnection& conn : logic->connections) {
		if (conn.fromNodeId == fromNodeId && conn.outputPin == outputPin) {
			nodes.push_back(conn.toNodeId);
		}
	}

	// no connection found
	return nodes;
}

LogicNode* VisualLogicSystem::findNode(VisualLC* logic, int nodeId) {
	for (LogicNode& node : logic->nodes) {
		if (node.id == nodeId) {
			return &node;
		}
	}

	return nullptr;
}

bool VisualLogicSystem::shouldTriggerCollision(LogicNode& node, GameObject* collided) {
	std::string filterType = node.parameters.count("colFilterType") ? node.parameters["colFilterType"] : "any";

	if (filterType == "any") return true;

	std::string filterValue = node.parameters.count("colFilterValue") ? node.parameters["colFilterValue"] : "";
	if (filterValue.empty()) return false;

	if (filterType == "name") {
		return collided->getObjectName() == filterValue;
	}
	else if (filterType == "tag") {
		CollisionInfo* col = collided->getComponent<CollisionInfo>();

		if (col) return col->collisionTag == filterValue;

		return false;
	}

	return false;
}