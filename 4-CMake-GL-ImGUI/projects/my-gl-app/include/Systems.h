#pragma once
#include <vector>
#include <fstream>
#include "Component.h"
#include "renderer.h" //RenderSystem needs the full definition of the renderer
#include "input.h" //InputSystem needs access to the InputHandler
#include "GameObjectManager.h"
#include <stb_image.h>

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
	GLuint uploadtex(std::string const& filename, bool& isTransparent);
};

class TransformSystem {
public:
	void update(GameObjectManager& manager, float deltaTime);
};