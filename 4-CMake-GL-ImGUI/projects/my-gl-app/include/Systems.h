/* Start Header ************************************************************************/
/*!
\file		Systems.h
\author
\par
\date		Month, Day, 2025
\brief
*/
/* End Header **************************************************************************/
#pragma once
#include <vector>
#include <fstream>
#include "Component.h"
#include "renderer.h" //RenderSystem needs the full definition of the renderer
#include "input.h" //InputSystem needs access to the InputHandler
#include "GameObjectManager.h"
#include <stb_image.h>
#include "collision.h"
#include <imgui.h> //for use of imgui in UISystem
#include <imgui_impl_glfw.h> //for use of imgui in UISystem
#include <imgui_impl_opengl3.h> //for use of imgui in UISystem
#include <ui.h>

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

class CollisionSystem {
public:
	void update(GameObjectManager& manager, float deltaTime);
};

class UISystem {
public:
	void update(GameObjectManager& manager, float deltaTime);
private:
	std::vector<std::string> debugMessages; //for messages
	std::vector<Collision::Circle> circles; 
};