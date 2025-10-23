/* Start Header ************************************************************************/
/*!
\file        Systems.cpp
\author      Seow Sin Le, s.sinle, 2401084, 45%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 15%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 25%
\author		 Asha Mathyalakan, asha.m, 2402886, 15%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\par		 p.lin@digipen.edu
\par		 asha.m@digipen.edu
\date        October, 1st, 2025
\brief       This file consists of System functions to run the logic for the Game Objects
			 and their components. 

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
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

// Input system - reads user input and updates velocity accordingly
void InputSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* iterator : gameObjects) {
		//check for objects that have input 
		if (iterator->hasComponent<Input>()) {
			Input* input = iterator->getComponent<Input>();
			if (iterator->hasComponent<Physics>()) {//player got that velocity in him
				Physics* velocity = iterator->getComponent<Physics>();


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

				if (InputHandler::isKeyTriggered(GLFW_KEY_0)) {
					UISystem::toggleUI();
				}


				// Press F9 to simulate a failure (for crash logger)
				if (InputHandler::isKeyTriggered(GLFW_KEY_F9)) {
					throw std::runtime_error("F9 test: simulated failure in update()");
				}
				
			}
		}
	}
}


// Render system - draws all game objects with a Render component
void RenderSystem::update(GameObjectManager& manager) {
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

			if (!render->isTransparent)
			{
				//position and orientation of mesh
				float angle = glm::radians(transform->rotation);
				transform->mdlWorld = glm::mat4{ 1 };
				transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
				transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
				transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));

				//texture bs
				if (render->hasTex)
				{
					glBindTextureUnit(0, render->texHDL);
				}
				else glBindTextureUnit(0, 0);
				render->wireframe = false;

				glm::mat4 texRot{ 1 };
				texRot = glm::rotate(texRot, angle, glm::vec3(0, 0, 1.f));
				glm::vec2 rotationCenter{ 0,0 }; //if needed in the future it can be a member variable, now just for sanity

				//binding model to world matrix
				glBindVertexArray(render->modelRef.vaoid);
				GLint vTransformLoc = glGetUniformLocation(renderer::shdr_pgm, "M");
				glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(transform->mdlWorld));

				//binding texture rotation and id
				GLint texid = glGetUniformLocation(renderer::shdr_pgm, "uTex2d");
				GLint rotmat = glGetUniformLocation(renderer::shdr_pgm, "uRotMtx");
				GLint rotcenter = glGetUniformLocation(renderer::shdr_pgm, "uMcn");
				GLint hastex = glGetUniformLocation(renderer::shdr_pgm, "uHasTex");
				GLint wireframe = glGetUniformLocation(renderer::shdr_pgm, "uIsWireframe");
				glUniform1i(texid, 0);
				glUniformMatrix2fv(rotmat, 1, GL_FALSE, glm::value_ptr(texRot));
				glUniform2fv(rotcenter, 1, glm::value_ptr(rotationCenter));
				glUniform1i(hastex, static_cast<int>(render->hasTex));
				glUniform1i(wireframe, static_cast<int>(render->wireframe));

				if (render->modelRef.shape == shape::square)
					glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
				else if (render->modelRef.shape == shape::circle)
					glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
				glBindVertexArray(0);
			}
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthMask(GL_FALSE);

	for (auto object : gameObjects) {
		if (object->hasComponent<Render>() && object->hasComponent<Transform>())
		{
			Transform* transform = object->getComponent<Transform>();
			Render* render = object->getComponent<Render>();

			if (render->isTransparent)
			{
				//position and orientation of mesh
				float angle = glm::radians(transform->rotation);
				transform->mdlWorld = glm::mat4{ 1 };
				transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
				transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
				transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));

				//texture bs
				if (render->hasTex)
				{
					glBindTextureUnit(0, render->texHDL);
				}
				else glBindTextureUnit(0, 0);
				render->wireframe = true;

				glm::mat4 texRot{ 1 };
				texRot = glm::rotate(texRot, angle, glm::vec3(0, 0, 1.f));
				glm::vec2 rotationCenter{ 0,0 }; //if needed in the future it can be a member variable, now just for sanity

				//binding model to world matrix
				glBindVertexArray(render->modelRef.vaoid);
				GLint vTransformLoc = glGetUniformLocation(renderer::shdr_pgm, "M");
				glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(transform->mdlWorld));

				//binding texture rotation and id
				GLint texid = glGetUniformLocation(renderer::shdr_pgm, "uTex2d");
				GLint rotmat = glGetUniformLocation(renderer::shdr_pgm, "uRotMtx");
				GLint rotcenter = glGetUniformLocation(renderer::shdr_pgm, "uMcn");
				GLint hastex = glGetUniformLocation(renderer::shdr_pgm, "uHasTex");
				GLint wireframe = glGetUniformLocation(renderer::shdr_pgm, "uIsWireframe");
				glUniform1i(texid, 0);
				glUniformMatrix2fv(rotmat, 1, GL_FALSE, glm::value_ptr(texRot));
				glUniform2fv(rotcenter, 1, glm::value_ptr(rotationCenter));
				glUniform1i(hastex, static_cast<int>(render->hasTex));
				glUniform1i(wireframe, 0);

				if (render->modelRef.shape == shape::square)
				{
					// --- Pass 1: textured fill ---
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glBindVertexArray(render->modelRef.vaoid);
					glUniform1i(wireframe, 0);
					glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
					// --- Pass 2: wireframe overlay ---
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glUniform1i(wireframe, 1);
					glLineWidth(2.0f);
					//glDepthMask(GL_FALSE);
					glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);

					// Restore state
					//glDepthMask(GL_TRUE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glUniform1i(wireframe, 0);
				}
					//glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
				else if (render->modelRef.shape == shape::circle)
					glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
				glBindVertexArray(0);
			}
		}
	}
	//glDepthMask(GL_TRUE);
}

// Load all textures for game objects with a Render component
void RenderSystem::init(GameObjectManager& manager)
{
	stbi_set_flip_vertically_on_load(true);
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Render>())
		{
			Render* render = object->getComponent<Render>();
			render->texHDL = uploadtex(render->texFile, render->isTransparent);
		}
	}
}

// Upload a texture from file and return its OpenGL handle
GLuint RenderSystem::uploadtex(std::string const& filename, bool& isTransparent)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!data) {
		std::cerr << "Failed to load image: " << filename << "]" << std::endl;
		return 0;
	}

	std::cout << "Loaded " << filename
		<< " (" << width << "x" << height << "), "
		<< channels << " channels" << std::endl;

	isTransparent = (channels == 4);

	GLuint texobj_hdl;
	glCreateTextures(GL_TEXTURE_2D, 1, &texobj_hdl);
	glTextureStorage2D(texobj_hdl, 1, GL_RGBA8, static_cast<GLuint>(width), static_cast<GLuint>(height));
	glTextureSubImage2D(texobj_hdl, 0, 0, 0, static_cast<GLuint>(width), static_cast<GLuint>(height), GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTextureParameteri(texobj_hdl, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texobj_hdl, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	stbi_image_free(data);
	return texobj_hdl;
}

// Physics system - updates position based on velocity and applies gravity
void PhysicsSystem::update(GameObjectManager& manager, const float& deltaTime) {

	// idk how physic programmer want to handle input for physics in the future
	// so here is a simple implementation for toggling step-by-step mode
	if (InputHandler::isComboKeyTriggered(GLFW_KEY_Z)) {
		m_stepMode = !m_stepMode;
	}
	if (m_stepMode && InputHandler::isKeyTriggered(GLFW_KEY_Z)) {
		m_stepReq = true;
	}

	// only update if NOT step-by-step physics debug mode or step requested
	if (m_stepMode && !m_stepReq) return;
	if (m_stepReq) m_stepReq = false; // reset step request after one update

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	GameObject* bullet = manager.getGameObject("bullet");

	for (auto object : gameObjects) {
		if (!object->hasComponent<Transform>() || !object->hasComponent<Physics>())
			continue;

		Transform* transform = object->getComponent<Transform>();
		Physics* physics = object->getComponent<Physics>();

		physics->velY += physics->gravity * deltaTime;

		if (object->hasComponent<Input>()) {
			//std::cout << object << " has input component\n";
			//bullet
			if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE)) {
				//std::cout << "bullet is spawned\n";
				UI::addDebugMessage("bullet is spawned");
				static int bulletCount = 0;

				Transform* t = bullet->getComponent<Transform>();
				t->x = transform->x;  
				t->y = transform->y;
				t->z = 0.02f;
				t->scaleX = 0.5f;
				t->scaleY = 0.5f;
				t->scaleZ = 0.5f;
				t->rotation = 0.0f;

				Physics* p = bullet->getComponent<Physics>();
				p->velX = 5.0f;
				p->velY = 5.0f;
				p->onGround = false;
			}

			// Jump
			if (InputHandler::isKeyTriggered(GLFW_KEY_B)){// && physics->onGround) {
				//std::cout << "player is jumping\n";
				UI::addDebugMessage("player is jumping");
				physics->velY = physics->jumpForce;
				//physics->velX = physics->Speed;
				physics->onGround = false;
			}
		}
		physics->velX *= physics->damping;

		transform->y += physics->velY * deltaTime;
		transform->x += physics->velX * deltaTime;

		if (transform->y < physics->floorY) {
			transform->y = physics->floorY;
			physics->velY = 0.0f;
			physics->onGround = true;
			physics->velX = 0.0f;
		}
		transform->x += physics->velX * deltaTime;
	}
}

// Collision system - detects and resolves collisions between game objects
void CollisionSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	//getting all objects
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	//create grid
	const float CELL_SIZE = 2.0f;
	const int GRID_WIDTH = 20;
	const int GRID_HEIGHT = 20;
	std::vector<std::vector<Collision::Cell>> grid(GRID_WIDTH, std::vector<Collision::Cell>(GRID_HEIGHT));


	for (GameObject* obj : gameObjects) {
		if (!obj->getComponent<CollisionInfo>() || !obj->hasComponent<Transform>()) continue;
		Transform* objT = obj->getComponent<Transform>();
		Collision::AABB box = Collision::getObjectAABB(objT);

		int minCellX = static_cast<int>(box.getMin().x / CELL_SIZE);
		int maxCellX = static_cast<int>(box.getMax().x / CELL_SIZE);
		int minCellY = static_cast<int>(box.getMin().y / CELL_SIZE);
		int maxCellY = static_cast<int>(box.getMax().y / CELL_SIZE);

		//clamp to grid boundaries so it dont go out the valid range of grid
		minCellX = std::max(0, std::min(minCellX, GRID_WIDTH - 1));
		maxCellX = std::max(0, std::min(maxCellX, GRID_WIDTH - 1));
		minCellY = std::max(0, std::min(minCellY, GRID_HEIGHT - 1));
		maxCellY = std::max(0, std::min(maxCellY, GRID_HEIGHT - 1));

		//putting the object into the grid cells it occupies
		for (int x = minCellX; x <= maxCellX; ++x) {
			for (int y = minCellY; y <= maxCellY; ++y) {
				grid[x][y].objects.push_back(obj);
			}
		}
	}

	//checking collision 
	for (int i = 0; i < GRID_WIDTH; i++) {
		for (int j = 0; j < GRID_HEIGHT; j++) {
			auto& cellObject = grid[i][j].objects; //get objects in the cell
			//loop through objects in the cell to check for collision
			for (size_t n = 0; n < cellObject.size(); n++) {
				GameObject* obj1 = cellObject[n];
				if (!obj1->hasComponent<Physics>()) continue;
				Transform* t1 = obj1->getComponent<Transform>();
				Collision::AABB aabb1 = Collision::getObjectAABB(t1);
				Physics* p1 = obj1->getComponent<Physics>();
				Vector2D vel1{ p1->velX, p1->velY };
				//object 2 looping
				for (size_t m = n + 1; m < cellObject.size(); m++) {
					GameObject* obj2 = cellObject[m];
					if (!obj2->hasComponent<Physics>()) continue;
					Transform* t2 = obj2->getComponent<Transform>();
					Collision::AABB aabb2 = Collision::getObjectAABB(t2);
					Physics* p2 = obj2->getComponent<Physics>();
					Vector2D vel2{ p2->velX, p2->velY };

					CollisionInfo info = Collision::CollisionIntersection_RectRect_Dynamic_Info(aabb1, vel1, aabb2, vel2); //check collision
					
					//checking if collided
					if (info.collided) {
						Collision::Resolve_AABB_AABB(
							aabb1, vel1, 0.0f,
							aabb2, vel2, 1.0f,
							info, CollisionResponseMode::Stop
						);

						//update velocities
						p1->velX = vel1.x;
						p1->velY = vel1.y;
						p2->velX = vel2.x;
						p2->velY = vel2.y;

						UI::addDebugMessage("Collision detected between Object.\n"); //message for debugger
					}
				}
			}
		}
	}
}

bool UISystem::showUI = false;

void UISystem::init(GLFWwindow* window) {
	// --- ImGui Setup ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.FontGlobalScale = 1.2f;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	// no show task bar icon for additional viewports
	io.ConfigViewportsNoTaskBarIcon = true;

	m_UI.loadAssetsFromDirectory();
}

UISystem::~UISystem() {
	// cleanup all imgui stuff here
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//for imgui
void UISystem::update(GameObjectManager& manager) {
	if (!showUI) return; // skip UI rendering if disabled

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	dockingSetUp();

	// all ImGui windows
	m_UI.update(manager);

	Transform* leavesT = gameObjects[0]->getComponent<Transform>();
	Transform* bulletT = gameObjects[2]->getComponent<Transform>();
	m_UI.shapeControl(*leavesT, *bulletT);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	renderMultipleViewPorts();
}

void UISystem::dockingSetUp() {
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None
		| ImGuiDockNodeFlags_PassthruCentralNode;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();
}

void UISystem::renderMultipleViewPorts() {
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}