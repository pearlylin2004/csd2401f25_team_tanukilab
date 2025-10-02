/* Start Header ************************************************************************/
/*!
\file        Systems.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 20%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 10%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\par		 p.lin@digipen.edu
\date        October, 1st, 2025
\brief       System functions to run the logic for the components

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
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

//all systems need to have an update function

// Input system - reads user input and updates velocity accordingly
class InputSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

// Physics system - updates position based on velocity and applies gravity
class PhysicsSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

// Render system - draws all game objects with a Render component
class RenderSystem {
public:
	void init(GameObjectManager& manager);
	void update(GameObjectManager& manager);
	GLuint uploadtex(std::string const& filename, bool& isTransparent);
};

// Transform system - updates the model matrix based on position, rotation, and scale
class TransformSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

// Collision system - detects and resolves collisions between game objects
class CollisionSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

// UI system - handles UI rendering and interactions using ImGui
class UISystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime, std::vector<Collision::Circle>& circles);
private:
	//std::vector<std::string> debugMessages; //for messages
	std::vector<Collision::Circle> circles; 
};