#pragma once

#include "systems.h"
#include "Component.h"
#include "input.h" //InputSystem needs access to the InputHandler
#include "renderer.h" //RenderSystem needs the full definition of the renderer
#include <vector>

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
			if (iterator->hasComponent<Velocity>()) {//player got that velocity in him
				Velocity* velocity = iterator->getComponent<Velocity>();

				//reset to 0 every frame
				velocity->dx = 0.0f;
				velocity->dy = 0.0f;

				if (InputHandler::isKeyDown(GLFW_KEY_W)) {
					velocity->dy = input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_S)) {
					velocity->dy = -input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_A)) {
					velocity->dx = -input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_D)) {
					velocity->dx = input->moveSpeed;
				}
				//so far so good i think?
			}
		}
	}
}
