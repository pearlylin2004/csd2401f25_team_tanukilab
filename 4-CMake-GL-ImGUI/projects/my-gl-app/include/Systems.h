#pragma once
#include "GameObjectManager.h"

//forward declaration
struct renderer;

class InputSystem {
public:
	void update(GameObjectManager& manager, float deltaTime);
};

class PhysicsSystem {
public:
	void update(GameObjectManager& manager, float deltaTime);
};

class RenderSystem {
public:
	void update(GameObjectManager& manager, float deltaTime);

};