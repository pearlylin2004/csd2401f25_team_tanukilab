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

std::map<std::string, float> VisualLC::globalFloatVar;
GameObjectManager* VisualLogicSystem::m_manager = nullptr;

void VisualLogicSystem::initAll(GameObjectManager& manager) {

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();
			if (!logic->justStarted) return;
			init(obj, logic);
		}
	}

	m_manager = &manager;
}

void VisualLogicSystem::updateAll(GameObjectManager& manager, float dt) {
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<GameObject*> gameObjects;

	manager.getAllGameObjects(gameObjects);


	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();

			/* -------- Check bullet refill event -------- */
			if (logic->bulletRefill) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::ON_BULLET_REFILL) {
						logic->currentExePath.clear();

						executeNode(obj, logic, node.id, dt);
					}
				}

				logic->bulletRefill = false;
			}
			/* -------- END -------- */


			update(obj, logic, dt);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Visual Script", ms });
}

void VisualLogicSystem::init(GameObject* obj, VisualLC* logic) {
	if (!obj || !logic) return;

	logic->hasHoverNodes = false;
	logic->hasClickNodes = false;

	for (LogicNode& node : logic->nodes) {
		// cache whether obj has hover nodes
		if (node.imp == NodeImplementation::ON_HOVER_ENTER ||
			node.imp == NodeImplementation::WHILE_HOVERING ||
			node.imp == NodeImplementation::ON_HOVER_EXIT) {
			logic->hasHoverNodes = true;
			//break;
		}

		if (node.imp == NodeImplementation::ON_CLICK_ENTER ||
			node.imp == NodeImplementation::WHILE_CLICKING ||
			node.imp == NodeImplementation::ON_CLICK_EXIT) {
			logic->hasClickNodes = true;
			//break;
		}

		// find and execute all ON START nodes
		if (node.imp == NodeImplementation::ON_START) {
			logic->currentExePath.clear();
			executeNode(obj, logic, node.id, 1 / 60.f);
		}
	}

	logic->hoverNodesChecked = true;
	logic->clickNodesChecked = true;
	logic->justStarted = false;
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
						//logic->stringVar["_collidedWith"] = collided->getObjectName();
						logic->lastCollidedObj = collided;
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
					//logic->stringVar["_collidedWith"] = collided->getObjectName();
					logic->lastCollidedObj = collided;
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
						//logic->stringVar["_collidedWith"] = collided->getObjectName();
						logic->lastCollidedObj = collided;
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


	/* -------- Safety Check jic haven check -------- */
	if (!logic->clickNodesChecked) {
		logic->hasClickNodes = false;
		for (const LogicNode& node : logic->nodes) {
			if (node.imp == NodeImplementation::ON_CLICK_ENTER ||
				node.imp == NodeImplementation::WHILE_CLICKING ||
				node.imp == NodeImplementation::ON_CLICK_EXIT) {
				logic->hasClickNodes = true;
				break;
			}
		}
		logic->clickNodesChecked = true;
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

	/* -------- Check click -------- */
	if (logic->hasClickNodes) {
		Transform* transform = obj->getComponent<Transform>();

		if (transform) {
			Vector2D mouseWorldPos = InputHandler::getWorldMousePosition();

#ifdef _DEBUG
			if (UISystem::isShowUI()) {
				ImVec2 scenePos; ImVec2 sceneSize;
				EditorManager::getScenePosNSize(scenePos, sceneSize);
				mouseWorldPos = InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize);
			}
#endif

			float halfW = transform->scaleX / 2.f;
			float halfH = transform->scaleY / 2.f;

			bool isOverObject = (mouseWorldPos.x >= transform->x - halfW &&
				mouseWorldPos.x <= transform->x + halfW &&
				mouseWorldPos.y >= transform->y - halfH &&
				mouseWorldPos.y <= transform->y + halfH);

			bool isClickedNow = isOverObject && InputHandler::isMouseLeftHeld();

			// ON CLICK ENTER
			if (isClickedNow && !logic->wasClickedLastFrame) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::ON_CLICK_ENTER) {
						logic->currentExePath.clear();
						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			// WHILE CLICKING
			if (isClickedNow) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::WHILE_CLICKING) {
						logic->currentExePath.clear();
						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			// ON CLICK EXIT
			if (!isClickedNow && logic->wasClickedLastFrame) {
				for (LogicNode& node : logic->nodes) {
					if (node.imp == NodeImplementation::ON_CLICK_EXIT) {
						logic->currentExePath.clear();
						executeNode(obj, logic, node.id, dt);
					}
				}
			}

			logic->wasClickedLastFrame = isClickedNow;
		}
	}
	/* -------- END -------- */


	/* -------- Check bullet shot event -------- */
	if (logic->bulletShot) {
		for (LogicNode& node : logic->nodes) {
			if (node.imp == NodeImplementation::ON_BULLET_SHOOT) {
				logic->currentExePath.clear();

				executeNode(obj, logic, node.id, dt);
			}
		}

		logic->bulletShot = false;
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

void VisualLogicSystem::broadcastBulletShoot() {
	std::vector<GameObject*> gameObjects;
	m_manager->getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (obj->hasComponent<VisualLC>()) {
			if (obj->hasComponent<VisualLC>()) {
				VisualLC* logic = obj->getComponent<VisualLC>();
				logic->bulletShot = true;
			}
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
	else if (node.imp == NodeImplementation::SET_ROTATION_RATE_OF_CHANGE) {
		Transform* transform = obj->getComponent<Transform>();
		if (transform) {
			float angle = node.parameters.count("angle") ? std::stof(node.parameters["angle"]) : 0.f;
			float rateOfChange = node.parameters.count("rate") ? std::stof(node.parameters["rate"]) : 0.f;
			float time = node.parameters.count("time") ? std::stof(node.parameters["time"]) : 0.f;
			transform->rotation = angle;
			static float timer = 0.f;
			static float initialChange = angle;
			if (timer < time && ((angle > 0 && initialChange > 0) || (angle < 0 && initialChange < 0)))
			{
				transform->rotation += initialChange * dt;
				initialChange += rateOfChange * dt;
				timer += dt;
			}
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

		std::cout << "\n\nSET_FLOAT Node ID: " << node.id << std::endl;
		std::cout << "  Variable: " << varName << std::endl;
		std::cout << "  Value from params: " << node.parameters["value"] << std::endl;
		std::cout << "  Parsed value: " << value << std::endl;

		logic->floatVar[varName] = value;
	}
	else if (node.imp == NodeImplementation::MODIFY_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		if (!logic->floatVar.count(varName)) {
			logic->floatVar[varName] = 0.f;
		}

		std::cout << "\n\nMODIFY_FLOAT Node ID: " << node.id << std::endl;
		std::cout << "  Variable: " << varName << std::endl;
		std::cout << "  Current value: " << logic->floatVar[varName] << std::endl;
		std::cout << "  Modify by: " << value << std::endl;

		logic->floatVar[varName] += value;
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

		// let audio component laoban control the looping or whatever
		// dont touch if already playing or pending to play
		if (channel->state == AudioState::Playing || channel->isPendingPlay) return;

		if (channel->state == AudioState::Stopped) {
			channel->isPendingPlay = true;
			channel->persistOnSceneChange = true;
			std::cout << "[VisualLogic] Triggering play for channel: " << channelName << std::endl;
		}
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
	else if (node.imp == NodeImplementation::SET_TEXTURE) {
		Render* render = obj->getComponent<Render>();

		if (!render) {
			DebugLog::addMessage("[VisualLogic] Object has no Render component!", DebugMode::PlaySimul);
			return;
		}

		std::string textureFile = node.parameters.count("textureFile") ?
			node.parameters["textureFile"] : "";

		if (textureFile == render->texFile) return;

		render->texFile = textureFile;
		render->hasTex = true;
		render->texChanged = true;
	}
	else if (node.imp == NodeImplementation::SET_POSITION_ON_COLLIDED) {
		GameObject* collidedObj = logic->lastCollidedObj;
		if (!collidedObj) return;

		Transform* collidedTransform = collidedObj->getComponent<Transform>();
		if (!collidedTransform) return;

		float deltaX = node.parameters.count("deltaXCollided") ? std::stof(node.parameters["deltaXCollided"]) : 0.f;
		float deltaY = node.parameters.count("deltaYCollided") ? std::stof(node.parameters["deltaYCollided"]) : 0.f;

		collidedTransform->x += deltaX;
		collidedTransform->y += deltaY;

		std::cout << "Set collided object's position to (" << collidedTransform->x << ", " << collidedTransform->y << ")\n";
	}
	else if (node.imp == NodeImplementation::SET_STATE_ON_COLLIDED) {
		GameObject* collidedObj = logic->lastCollidedObj;
		if (!collidedObj) return;

		StateMachine* collidedSM = collidedObj->getComponent<StateMachine>();
		if (!collidedSM) return;

		std::string stateName = node.parameters.count("targetStateCollided") ? node.parameters["targetStateCollided"] : JsonIO::stateNames[0];

		collidedSM->state = JsonIO::parsePlayerState(stateName);
		collidedSM->stateTime = 0.f;
		collidedSM->fixState = true; // avoid getting overridden by statemachine 

		//std::cout << "Set collided object's state to " << stateName << "\n";
	}
	else if (node.imp == NodeImplementation::APPLY_FORCE_ON_COLLIDED) {
		GameObject* collidedObj = logic->lastCollidedObj;
		if (!collidedObj) return;

		Physics* collidedPhysic = collidedObj->getComponent<Physics>();
		if (!collidedPhysic) return;

		float forceX = node.parameters.count("forceX") ? std::stof(node.parameters["forceX"]) : 0.f;
		float forceY = node.parameters.count("forceY") ? std::stof(node.parameters["forceY"]) : 0.f;

		collidedPhysic->dynamics.velocity.x += forceX;
		collidedPhysic->dynamics.velocity.y += forceY;

		//std::cout << "Put force on collided object to (" << collidedPhysic->dynamics.velocity.x << ", " << collidedPhysic->dynamics.velocity.y << ")\n";
	}
	else if (node.imp == NodeImplementation::SET_GLOBAL_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		VisualLC::globalFloatVar[varName] = value;
	}
	else if (node.imp == NodeImplementation::MODIFY_GLOBAL_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		float value = node.parameters.count("value") ?
			std::stof(node.parameters["value"]) : 0.f;

		VisualLC::globalFloatVar[varName] += value;
	}
	else if (node.imp == NodeImplementation::TRIGGER_BULLET_REFILL_EVENT) {
		std::string targetType = node.parameters.count("targetType") ?
			node.parameters["targetType"] : "all";

		std::string targetValue = node.parameters.count("targetValue") ?
			node.parameters["targetValue"] : "";

		// broadcast to all objects
		if (targetType == "all") {
			std::vector<GameObject*> gameObjects;
			m_manager->getAllGameObjects(gameObjects);

			for (GameObject* targetObj : gameObjects) {
				if (targetObj->hasComponent<VisualLC>()) {
					targetObj->getComponent<VisualLC>()->bulletRefill = true;
				}
			}

		}
		// trigger only on collided obj
		else if (targetType == "collided") {
			if (!logic->lastCollidedObj) return;

			if (logic->lastCollidedObj->hasComponent<VisualLC>())
				logic->lastCollidedObj->getComponent<VisualLC>()->bulletRefill = true;
		}
		else if (targetType == "tag") {
			if (targetValue.empty()) return;

			std::vector<GameObject*> gameObjects;
			m_manager->getAllGameObjects(gameObjects);

			for (GameObject* object : gameObjects) {
				if (object->hasComponent<CollisionInfo>() && object->hasComponent<VisualLC>()) {

					CollisionInfo* col = object->getComponent<CollisionInfo>();
					if (col->collisionTag == targetValue) {
						object->getComponent<VisualLC>()->bulletRefill = true;
					}
				}
			}
		}
		else if (targetType == "name") {
			if (targetValue.empty()) return;

			std::vector<GameObject*> gameObjects;
			m_manager->getAllGameObjects(gameObjects);

			for (GameObject* object : gameObjects) {
				if (object->getObjectName() == targetValue && object->hasComponent<VisualLC>())
					object->getComponent<VisualLC>()->bulletRefill = true;
			}
		}
		else if (targetType == "self") {
			logic->bulletRefill = true;
		}
	}
	else if (node.imp == NodeImplementation::RESTART_LEVEL) {
		GUISystem::getInstance().m_pendingRestart = true;
	}
	else if (node.imp == NodeImplementation::LOAD_LEVEL) {
		std::string levelName = node.parameters.count("levelName") ?
			node.parameters["levelName"] : "level01.json";
		GUISystem::getInstance().m_pendingLevelLoad = true;
		GUISystem::getInstance().m_pendingLevelName = levelName;
	}
	else if (node.imp == NodeImplementation::DISABLE_COLLISION) {
		CollisionInfo* collision = obj->getComponent<CollisionInfo>();
		if (collision)
			collision->collisionFlag = false;
	}
	/*else if(node.imp == NodeImplementation::ENABLE_COLLISION) {
		CollisionInfo* collision = obj->getComponent<CollisionInfo>();
		if (collision) {
			collision->enabled = true;
		}
	}*/
	else if (node.imp == NodeImplementation::IS_SINLE_PHYSICS) {
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->sinlePersonalBool = true;
			//std::cout << "\n\n Is SinLe Physics\n";
			//std::cout << "sinle bool is now true\n";
		}
		}
	else if (node.imp == NodeImplementation::ENABLE_PHYSICS) {
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->sinleFlag = true;
			//std::cout << "sinle flag is now true\n";
		}
	}
	else if(node.imp == NodeImplementation::DISABLE_PHYSICS) {
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->sinleFlag = false;
		}
	}
	else if (node.imp == NodeImplementation::ENTER_WATER)
	{
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->hitWater = true;
			//std::cout << "obj has entered water\n";
		}
	}
	else if (node.imp == NodeImplementation::EXIT_WATER)
	{
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->inWater = false;
		}
	}
	else if(node.imp == NodeImplementation::HIT_GROUND)
		{
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->hitground = true;
		}
	}
	else if(node.imp == NodeImplementation::NOT_ON_GROUND)
	{
		if (obj->hasComponent<Physics>())
		{
			Physics* physics = obj->getComponent<Physics>();
			physics->onGround = false;
		}
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

		return logic->timers[timerName] > threshold;
	}
	else if (node.imp == NodeImplementation::COMPARE_GLOBAL_FLOAT) {
		std::string varName = node.parameters.count("variable") ?
			node.parameters["variable"] : "var";

		std::string comparison = node.parameters.count("comparison") ?
			node.parameters["comparison"] : "greater";

		float threshold = node.parameters.count("threshold") ?
			std::stof(node.parameters["threshold"]) : 0.f;

		// get variable value
		float currentValue = 0.f;
		if (VisualLC::globalFloatVar.count(varName)) {
			currentValue = VisualLC::globalFloatVar[varName];
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
	else if (node.imp == NodeImplementation::CHECK_COLLIDED_STATE) {
		GameObject* collidedObj = logic->lastCollidedObj;
		if (!collidedObj) {
			DebugLog::addMessage("[VisualLogic] No collided object to check state!", DebugMode::PlaySimul);
			return false;
		}

		StateMachine* collidedSM = collidedObj->getComponent<StateMachine>();
		if (!collidedSM) {
			DebugLog::addMessage("[VisualLogic] Collided object has no StateMachine component!", DebugMode::PlaySimul);
			return false;
		}

		std::string target = node.parameters.count("state") ? node.parameters["state"] : JsonIO::stateNames[0];

		return JsonIO::parsePlayerState(target) == collidedSM->state;
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

void VisualLogicSystem::resetLogicState(/*GameObjectManager& manager*/) {
	std::vector<GameObject*> allObjects;
	m_manager->getAllGameObjects(allObjects);

	VisualLC::globalFloatVar.clear();

	// Loop through all objects and reset their VisualLC components
	for (GameObject* obj : allObjects) {
		if (obj->hasComponent<VisualLC>()) {
			VisualLC* logic = obj->getComponent<VisualLC>();


			logic->justStarted = true;           // Tell system this is a fresh start
			logic->isExecuting = false;          // Not currently executing
			logic->currentExecutingNodeId = -1;  // No node is executing

			logic->intVar.clear();       // Clear all integer variables
			logic->floatVar.clear();     // Clear all float variables
			logic->stringVar.clear();    // Clear all string variables
			logic->boolVar.clear();      // Clear all boolean variables
			logic->timers.clear();       // Clear all timers (they'll restart from 0)
			logic->currentExePath.clear(); // Clear execution path tracking

			logic->wasHoveredLastFrame = false; // Not being hovered

			logic->wasClickedLastFrame = false;
			logic->hasClickNodes = false;
			logic->clickNodesChecked = false;

			logic->curCollision.clear();  // Clear current collisions
			logic->preCollision.clear();  // Clear previous collisions
			logic->lastCollidedObj = nullptr; // No last collided object

			logic->bulletShot = false;   // No bullet shot event
			logic->bulletRefill = false; // No bullet refill event

		}
	}

}