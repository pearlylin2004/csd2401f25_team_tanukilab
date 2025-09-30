/* Start Header ************************************************************************/
/*!
\file		Systems.cpp
\author
\par
\date		Month, Day, 2025
\brief
*/
/* End Header **************************************************************************/
#pragma once

#include "Systems.h"


//here we go buddies

//all the systems need to get all game objects
//then it needs to check through the list of 
//game objects to check if the valid component
//is present or not
//so all systems need to use the getAllGameObjects() function
void InputSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* iterator : gameObjects) {
		//check for objects that have input 
		if (iterator->hasComponent<Input>()) {
			Input* input = iterator->getComponent<Input>();
			if (iterator->hasComponent<Physics>()) {//player got that velocity in him
				Physics* velocity = iterator->getComponent<Physics>();

				//reset to 0 every frame
				velocity->velX = 0.0f;
				velocity->velY = 0.0f;

				if (InputHandler::isKeyHeld(GLFW_KEY_W)) {
					velocity->velY = input->moveSpeed;
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_S)) {
					velocity->velY = -input->moveSpeed;
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
					velocity->velX = -input->moveSpeed;
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_D)) {
					velocity->velX = input->moveSpeed;
				}
				//so far so good i think?
			}
		}
	}
}

void RenderSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	renderer::cam.update();
	glUseProgram(renderer::shdr_pgm);
	GLint vTransformView = glGetUniformLocation(renderer::shdr_pgm, "V");
	//renderer::cam.view = glm::mat4{ 1 };
	glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(renderer::cam.view));
	GLint vTransformProj = glGetUniformLocation(renderer::shdr_pgm, "P");
	//renderer::cam.proj = glm::mat4{ 1 };
	glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(renderer::cam.proj));

	for (auto object : gameObjects) {
		if (object->hasComponent<Render>() && object->hasComponent<Transform>())
		{
			Transform* transform = object->getComponent<Transform>();
			Render* render = object->getComponent<Render>();
			float angle = glm::radians(transform->rotation);
			transform->mdlWorld = glm::mat4{ 1 };
			transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
			transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
			transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));

			glBindVertexArray(render->modelRef.vaoid);
			GLint vTransformLoc = glGetUniformLocation(renderer::shdr_pgm, "M");
			glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(transform->mdlWorld));

			if (render->modelRef.shape == shape::square)
				glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
			else if (render->modelRef.shape == shape::circle)
				glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
			glBindVertexArray(0);
		}
	}
}

void PhysicsSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (auto object : gameObjects) {
		if (object->hasComponent<Transform>() && object->hasComponent<Physics>()) {
			Transform* transform = object->getComponent<Transform>();
			Physics* physics = object->getComponent<Physics>();
			transform->x += physics->velX * deltaTime;
			transform->y += physics->velY * deltaTime;


			/*if (object->hasComponent<Physics>() && !object->hasComponent<Input>()) {
				*/
				/*Physics* physics = object->getComponent<Physics>();*/

			if (InputHandler::isKeyHeld(GLFW_KEY_B)) { ///Needa update
				if (!physics->inWaterTriggered) {
					physics->velY = physics->waterBounce;     // instant upward push
					physics->inWaterTriggered = true;
				}
			}

			if (physics->inWaterTriggered) {
				// Apply gravity and damping only after entering water
				physics->velY += (physics->gravity * 0.3f) * deltaTime;
				physics->velY *= physics->damping;

				transform->y += physics->velY * deltaTime;

				// Floor collision
				if (transform->y < physics->floorY) {
					transform->y = physics->floorY;
					physics->velY = 0.0f;

					physics->inWaterTriggered = false;
				}
			}
		}
	}
}

void CollisionSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* iterator : gameObjects) {
		if (iterator->hasComponent<CollisionInfo>() && iterator->hasComponent<Physics>()) {

			for (GameObject* obj : gameObjects) {
				if (obj != iterator) {
					Transform* obj1T = obj->getComponent<Transform>();
					Transform* obj2T = iterator->getComponent<Transform>();
					Collision::AABB obj1 = Collision::getObjectAABB(obj1T);
					Collision::AABB obj2 = Collision::getObjectAABB(obj2T);
					Physics* obj1V = obj->getComponent<Physics>();
					Physics* obj2V = iterator->getComponent<Physics>();
					Vector2D obj1Vel{ obj1V->velX, obj1V->velY };
					Vector2D obj2Vel{ obj2V->velX, obj2V->velY };
					CollisionInfo info = Collision::CollisionIntersection_RectRect_Dynamic_Info(obj1, obj1Vel, obj2, obj2Vel);

					if (info.collided) {
						Collision::Resolve_AABB_AABB(
							obj1, obj1Vel, 0.0f,
							obj2, obj2Vel, 1.0f,
							info, CollisionResponseMode::Stop
						);

						//change velocity
						obj1V->velX = obj1Vel.x;
						obj1V->velY = obj1Vel.y;
						obj2V->velX = obj2Vel.x;
						obj2V->velY = obj2Vel.y;
					}
				}
			}
		}
	}
}