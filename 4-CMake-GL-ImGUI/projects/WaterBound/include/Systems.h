/* Start Header ************************************************************************/
/*!
\file        Systems.h
\author      Seow Sin Le, s.sinle, 2401084, 40%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 40%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 20%
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
#include "imgui_internal.h" // for docking in UISystem
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

private:
	bool m_stepMode = false;
	bool m_stepReq = false;
};

/*!***********************************************************************
\brief
	render component

*************************************************************************/
class RenderSystem {
	GLuint fbo;
	GLuint texture;

	int fboWidth = 0;
	int fboHeight = 0;

	void createFBO();
public:
	/*!***********************************************************************
	\brief
		initializes textures for all objects with render component

	\param[in] manager
		array of all objects

	*************************************************************************/
	void init(GameObjectManager& manager, int fboW, int fboH);
	/*!***********************************************************************
	\brief
		resend matrix data to the shaders to recalculated object position and orientation to draw for the new frame

	\param[in] manager
		array of all objects

	*************************************************************************/
	void update(GameObjectManager& manager, float const& deltaTime);
	GLuint uploadtex(std::string const& filename, bool& isTransparent);
	void renderTex(GameObject* object, float const& deltaTime);
	void renderNoTex(GameObject* object);

	GLuint getTexture() const { return texture; }

	void fboAspectRatio(int& width, int& height) const;
	~RenderSystem();
};

/*!***********************************************************************
\brief
	transform component

*************************************************************************/
class TransformSystem {
public:
	/*!***********************************************************************
	\brief
		updating the new position, scale and orientation of the object

	\param[in] GameObjectManager manager
		array of all objects

	\param[in] deltaTime
		tracking of the world time

	*************************************************************************/
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
	void init(GLFWwindow* window, RenderSystem* renderer);
	~UISystem();
	void update(GameObjectManager& manager);

	static void toggleUI() { showUI = !showUI; } // temp toggle function
	static bool isShowUI() { return showUI; }

private:
	void dockingSetUp();
	void renderMultipleViewPorts();
	UI m_UI;
	static bool showUI;
	bool m_firstTime = true;
	RenderSystem* m_renderer = nullptr;
};