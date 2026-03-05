/* Start Header ************************************************************************/
/*!
\file        Systems.cpp
\author      Seow Sin Le, s.sinle, 2401084, 40%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 15%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 10%
\author		 Asha Mathyalakan, asha.m, 2402886, 15%
\author		 Tan Chee Qing, cheeqing.tan, 2401486, 20%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\par		 p.lin@digipen.edu
\par		 asha.m@digipen.edu
\par		 cheeqing.tan@digipen.edu
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
#include "Physics.h"

std::string TileMapSystem::filename{};
std::vector<Particle::UID> Particle::UIDs;
std::map<BatchKey, std::vector<renderer::InstanceData>> RenderSystem::objectWithTex{};
std::map<BatchKey, std::vector<renderer::InstanceData>> RenderSystem::objectWithoutTex{};
//here we go buddies

//all the systems need to get all game objects
//then it needs to check through the list of 
//game objects to check if the valid component
//is present or not
//so all systems need to use the getAllGameObjects() function
//JumpForce jumpForce;
//BulletForce bulletForce;
// Input system - reads user input and updates velocity accordingly
void InputSystem::update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus) {
    (void)deltaTime;
    (void)messageBus;
    //record current time for performance tracking
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

#ifdef _DEBUG
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_0)) {
        UISystem::toggleUI();

        // refresh cached object vector
        CachedGameObjects::Instance().refresh();
		GUISystem::getInstance().createButtons(/*manager*/);

        messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "0", true }));

        if (EditorManager::isEditingMode()) EditorManager::toggleEditing(false);
        else EditorManager::toggleEditing(true);
    }

    // ============ LAYER TESTING CODE ============
    // Press Ctrl+1 to toggle Background layer (layer 0)
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_1)) {
        LayerManager& lm = manager.getLayerManager();
        bool currentState = lm.isLayerEnabled(0);
        lm.setLayerEnabled(0, !currentState);
        std::cout << "Background layer " << (!currentState ? "ENABLED" : "DISABLED") << std::endl;
    }

    // Press Ctrl+2 to toggle Game layer (layer 1)
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_2)) {
        LayerManager& lm = manager.getLayerManager();
        bool currentState = lm.isLayerEnabled(1);
        lm.setLayerEnabled(1, !currentState);
        std::cout << "Game layer " << (!currentState ? "ENABLED" : "DISABLED") << std::endl;
    }

    // Press Ctrl+3 to toggle UI layer (layer 2)
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_3)) {
        LayerManager& lm = manager.getLayerManager();
        bool currentState = lm.isLayerEnabled(2);
        lm.setLayerEnabled(2, !currentState);
        std::cout << "UI layer " << (!currentState ? "ENABLED" : "DISABLED") << std::endl;
    }

    // Press Ctrl+4 to toggle Game layer visibility only (still updates, just invisible)
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_4)) {
        LayerManager& lm = manager.getLayerManager();
        Layer* gameLayer = lm.getLayer(1);
        if (gameLayer) {
            bool currentVisible = gameLayer->isVisible();
            gameLayer->setVisible(!currentVisible);
            std::cout << "Game layer visibility " << (!currentVisible ? "ON" : "OFF") << std::endl;
        }
    }

    // Press Ctrl+5 to toggle Game layer updates only (visible but frozen)
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_5)) {
        LayerManager& lm = manager.getLayerManager();
        Layer* gameLayer = lm.getLayer(1);
        if (gameLayer) {
            bool currentUpdatable = gameLayer->isUpdatable();
            gameLayer->setUpdatable(!currentUpdatable);
            std::cout << "Game layer updates " << (!currentUpdatable ? "ON" : "OFF") << std::endl;
        }
    }

    // Press Ctrl+6 to print layer status
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_6)) {
        LayerManager& lm = manager.getLayerManager();
        std::cout << "\n=== LAYER STATUS ===" << std::endl;
        for (Layer* layer : lm.getAllLayers()) {
            if (layer) {
                std::cout << "Layer " << layer->getLayerID() << " (" << layer->getLayerName() << "): "
                          << "Enabled=" << (layer->isEnabled() ? "YES" : "NO")
                          << ", Visible=" << (layer->isVisible() ? "YES" : "NO")
                          << ", Updatable=" << (layer->isUpdatable() ? "YES" : "NO")
                          << ", Objects=" << layer->getObjectCount() << std::endl;
            }
        }
        std::cout << "===================\n" << std::endl;
    }
  
#endif

    // Press F9 to simulate a failure (for crash logger)
    if (InputHandler::isKeyTriggered(GLFW_KEY_F9)) {
        throw std::runtime_error("F9 test: simulated failure in update()");
    }

    /* --- for showing fps scuffed scuffed ---- */
    if (InputHandler::isKeyTriggered(GLFW_KEY_F8)) {
        FontSystem::showFPS = !FontSystem::showFPS;
    }
    /* ---- END ---- */

    // Press 8 to manually run FSM assignment check
	if (InputHandler::isComboKeyTriggered(GLFW_KEY_8)) {
        std::cout << "\n[DEBUG KEY 8] ====== MANUAL FSM ASSIGNMENT CHECK ======\n";
        
        std::vector<GameObject*> allObjects;
        manager.getAllGameObjects(allObjects);
        
        for (GameObject* obj : allObjects) {
            if (!obj) continue;
            
            std::string name = obj->getObjectName();
            CollisionInfo* col = obj->getComponent<CollisionInfo>();
            StateMachine* sm = obj->getComponent<StateMachine>();
            Input* inp = obj->getComponent<Input>();
            Physics* phys = obj->getComponent<Physics>();
            Animation* anim = obj->getComponent<Animation>();
            
            std::string tag = col ? col->collisionTag : "(no CollisionInfo)";
            
            std::cout << "[DEBUG] Object: " << name 
                      << " | Layer: " << obj->getLayer()
                      << " | Tag: " << tag
                      << " | StateMachine: " << (sm ? "YES" : "NO")
                      << " | Input: " << (inp ? "YES" : "NO")
                      << " | Physics: " << (phys ? "YES" : "NO")
                      << " | Animation: " << (anim ? "YES" : "NO")
                      << "\n";
            
            // Check if this object SHOULD have an FSM
            if (col && col->collisionTag == "player") {
                std::cout << "       -> Player object detected!\n";
                if (!sm) std::cout << "       -> WARNING: Missing StateMachine component!\n";
                if (!inp) std::cout << "       -> WARNING: Missing Input component!\n";
            }
        }

        // Specifically check for player
       /* GameObject* player = manager.getGameObject("player");
        if (player) {
            std::cout << "\n[DEBUG] PLAYER FOUND!\n";
            std::cout << "  - Has StateMachine: " << (player->hasComponent<StateMachine>() ? "YES" : "NO") << "\n";
            std::cout << "  - Has Input: " << (player->hasComponent<Input>() ? "YES" : "NO") << "\n";
            std::cout << "  - Has Physics: " << (player->hasComponent<Physics>() ? "YES" : "NO") << "\n";
            std::cout << "  - Has Animation: " << (player->hasComponent<Animation>() ? "YES" : "NO") << "\n";
            
            CollisionInfo* col = player->getComponent<CollisionInfo>();
            if (col) {
                std::cout << "  - Collision Tag: " << col->collisionTag << "\n";
            }
        } else {
            std::cout << "\n[DEBUG] WARNING: No object named 'player' found!\n";
        }*/
        
        std::cout << "[DEBUG KEY 8] ======================================\n\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    g_SystemTimers.push_back({ "Input", ms });
}

// Render system - draws all game objects with a Render component
void RenderSystem::update(GameObjectManager& manager, float const& deltaTime) 
{
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();
	batchingSetUp(manager, deltaTime);

	//we aint doing this anymore 
	//std::vector<GameObject*> gameObjects;
	//manager.getAllGameObjects(gameObjects);

	LayerManager& layerManager = manager.getLayerManager();
	std::vector<Layer*> layers = layerManager.getAllLayers();

	glm::mat4 camView, camProj;

	// decide where to render
	renderFBO();

	// Attach texture to FBO and update editor camera if editor is on & update camera
	if (UISystem::isShowUI() && EditorManager::isEditingMode())
	{
		createFBO();
		renderer::editorCam.update();

		camView = renderer::editorCam.view;
		camProj = renderer::editorCam.proj;
	}
	else
	{
		renderer::cam.update();

		camView = renderer::cam.view;
		camProj = renderer::cam.proj;
	}

	for (Layer* layer : layers) {
		if (!layer) continue;
		
		//gettem objects bro
		const std::vector<GameObject*>& gameObjects = layer->getObjects();
		for (GameObject* object : gameObjects) {
			if (object->hasComponent<Render>()) {
				Render* render = object->getComponent<Render>();

				if (render->texChanged) {
					if (render->hasTex) {
						TextureData texData = ResourceManager::getInstance().getTexture(render->texFile);
						render->texHDL = texData.id;
						render->isTransparent = texData.isTransparent;
						render->texChanged = false;
					}

					TextureData texData = ResourceManager::getInstance().getTexture(render->texFile);
					render->texHDL = texData.id;
					render->isTransparent = texData.isTransparent;
					render->texChanged = false;
				}

				if (object->hasComponent<Animation>()) {
					Animation* animation = object->getComponent<Animation>();
					if (!animation->animState.empty())
					{
						for (AnimateState& as : animation->animState) {
							if (as.texChanged && !as.texFile.empty()) {
								TextureData textureData = ResourceManager::getInstance().getTexture(as.texFile);
								as.texHDL = textureData.id;
								as.texChanged = false;
							}
						}
					}
				}

			}
		}


	}
	//drawing models without texture
	glDepthMask(GL_TRUE);
	glUseProgram(renderer::shdr_pgm[1]);
	GLint uView = glGetUniformLocation(renderer::shdr_pgm[1], "V");
	GLint uProj = glGetUniformLocation(renderer::shdr_pgm[1], "P");
	glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(camView));
	glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(camProj));
	instancePerBatch(objectWithoutTex);
	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithoutTex)
	{
		renderer::drawInstances(pair);
	}
	//glDisable(GL_DEPTH_TEST);
	//drawing models with texture
	glDepthMask(GL_FALSE);  // don't write to depth buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(renderer::shdr_pgm[0]); // hasTex shader
	uView = glGetUniformLocation(renderer::shdr_pgm[0], "V");
	uProj = glGetUniformLocation(renderer::shdr_pgm[0], "P");
	glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(camView));
	glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(camProj));
	instancePerBatch(objectWithTex);
	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex)
	{
		const BatchKey& key = pair.first;
		//std::vector<renderer::InstanceData>& instances = pair.second;
		glBindTextureUnit(0, key.texID);
		GLint uTexLoc = glGetUniformLocation(renderer::shdr_pgm[0], "uTex2d");
		if (uTexLoc != -1) 
			glUniform1i(uTexLoc, 0);
		//renderer::model& mdl = renderer::models[(int)key.meshType];
		renderer::drawInstances(pair);
	}
	glDepthMask(GL_TRUE);
	//instancePerBatch(objectWithTex2);
	//for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex2)
	//{
	//	const BatchKey& key = pair.first;
	//	//std::vector<renderer::InstanceData>& instances = pair.second;
	//	glBindTextureUnit(0, key.texID);
	//	GLint uTexLoc = glGetUniformLocation(renderer::shdr_pgm[0], "uTex2d");
	//	if (uTexLoc != -1)
	//		glUniform1i(uTexLoc, 0);
	//	//renderer::model& mdl = renderer::models[(int)key.meshType];
	//	renderer::drawInstances(pair);
	//}

	/*if (UISystem::isShowUI()) glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	//record end time for performance tracking
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({"Render", ms }); //saving timing for UI output
}

void RenderSystem::renderFBO() {
	if (!UISystem::isShowUI()) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // render directly to screen
	}
	else {
		createFBO();
	}
}

void RenderSystem::resizeFBO(int width, int height) {
	fboWidth = width;
	fboHeight = height;

	if (fbo) {
		if (texture) glDeleteTextures(1, &texture);
		if (fbo) glDeleteFramebuffers(1, &fbo);
		if (depthBuffer) glDeleteRenderbuffers(1, &depthBuffer);
		fbo = 0;  // reset so createFBO will recreate it
	}
}

void RenderSystem::createFBO() {
	// only create FBO once if its not created
	if (!fbo) {
		/* --- Create and set up a framebuffer object (FBO) --- */
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		/* --- Create and colour texture attachment for fbo --- */
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		/* --- Create a depth and stencil renderbuffer --- */
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fboWidth, fboHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	/* --- Clear the FBO's color and depth buffers before rendering  --- */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

RenderSystem::~RenderSystem() {
	if (texture) glDeleteTextures(1, &texture);
	if (fbo) glDeleteFramebuffers(1, &fbo);
	if (depthBuffer) glDeleteRenderbuffers(1, &depthBuffer);
}

// Load all textures for game objects with a Render component
void RenderSystem::init(GameObjectManager& manager, int fboW, int fboH)
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//stbi_set_flip_vertically_on_load(true);
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Render>()) {
			Render* render = object->getComponent<Render>();
			if (object->getComponent<Render>()->hasTex)
			{
				//render->texHDL = uploadtex(render->texFile, render->isTransparent);
				TextureData textureData = ResourceManager::getInstance().getTexture(render->texFile);
				render->texHDL = textureData.id;
				render->isTransparent = textureData.isTransparent;
				//std::cerr << "Texture ID for " << render->texFile << ": " << render->texHDL << std::endl;
				//std::cerr << "Is Transparent: " << (render->isTransparent ? "Yes" : "No") << std::endl;
			}
		}

		if (object->hasComponent<Animation>()) {
			Animation* animation = object->getComponent<Animation>();
			if (!animation->animState.empty())
			{
				for (AnimateState& as : animation->animState) {
					if (!as.texFile.empty()) {
						TextureData textureData = ResourceManager::getInstance().getTexture(as.texFile);
						as.texHDL = textureData.id;
					}
				}
			}
		}
	}
	fboWidth = fboW;
	fboHeight = fboH;
}

void RenderSystem::batchingSetUp(GameObjectManager& manager, float const& deltaTime)
{
	LayerManager& layerManager = manager.getLayerManager();
	
	//for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex) pair.second.clear();
	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithoutTex) pair.second.clear();

	// Process objects layer by layer for efficient visibility check
	for (Layer* layer : layerManager.getAllLayers()) {
		if (!layer || !layer->isVisible()) continue; // Skip non-visible layers (single flag check)

		for (GameObject* obj : layer->getObjects()) {
			if (!obj->hasComponent<Render>() || !obj->hasComponent<Transform>())
				continue;

			Render* render = obj->getComponent<Render>();
			Transform* transform = obj->getComponent<Transform>();

			float scaleX = transform->flipX ? -transform->scaleX : transform->scaleX;

			renderer::InstanceData data;
			float angle = glm::radians(transform->rotation);
			transform->mdlWorld = glm::mat4{ 1 };
			transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
			transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
			transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(scaleX, transform->scaleY, transform->scaleZ));
			data.model = transform->mdlWorld;
			data.color = render->clr;
			data.texParams = glm::vec4(0, 0, 1, 1);

			// Animation handling (same as before)
			if (obj->hasComponent<Animation>() && obj->hasComponent<StateMachine>())
			{
				Animation* animation = obj->getComponent<Animation>();
				StateMachine* sm = obj->getComponent<StateMachine>();
				AnimateState& as = animation->animState[static_cast<int>(sm->state)];

				if (render->hasAnimation && !as.texFile.empty()) {
					glm::vec2 texOffSet{ as.currentFrameColumn / static_cast<float>(as.totalColumn), 1.f - ((as.currentFrameRow + 1) / static_cast<float>(as.totalRow)) };
					glm::vec2 texScale{ 1.f / as.totalColumn, 1.f / as.totalRow };

					// Only update animation if layer is updatable
					if (as.play && layer->isUpdatable() && !EditorManager::isEditingMode() && !EditorManager::isPaused())
					{
						as.frameTimer += deltaTime;
						if (as.frameTimer >= as.frameTime)
						{
							as.frameTimer -= as.frameTime;
							if (as.currentFrameColumn >= as.lastFrame.x && as.currentFrameRow >= as.lastFrame.y)
							{
								if (as.loop) {
									as.currentFrameColumn = (int)as.initialFrame.x;
									as.currentFrameRow = (int)as.initialFrame.y;
									as.frameTimer = 0;
								}
								else {
									as.play = false;
								}
							}
							else {
								++as.currentFrameColumn;
								if (as.currentFrameColumn >= as.totalColumn) {
									as.currentFrameColumn = 0;
									++as.currentFrameRow;
									if (as.currentFrameRow > as.lastFrame.y) {
										as.currentFrameColumn = (int)as.initialFrame.x;
										as.currentFrameRow = (int)as.initialFrame.y;
									}
								}
							}
							animation->runItBack = true;
						}
					}
					data.texParams = { texOffSet, texScale };
					//data.textureID = render->texHDL;
					BatchKey key{ transform->z, as.texHDL, render->modelRef.shape };
					objectWithTex[key].push_back(data);
				}
				else {
					BatchKey key{ transform->z, 0, render->modelRef.shape };
					objectWithoutTex[key].push_back(data);
				}
			}
			else if (render->hasTex) {
				//data.textureID = render->texHDL;
				BatchKey key{ transform->z, render->texHDL, render->modelRef.shape };
				objectWithTex[key].push_back(data);
			}
			else {
				BatchKey key{ transform->z, 0, render->modelRef.shape };
				objectWithoutTex[key].push_back(data);
			}
		}
	}
}

//void RenderSystem::batchingSetUp(GameObjectManager& manager, float const& deltaTime)
//{
//	LayerManager& layerManager = manager.getLayerManager();
//	
//	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex) pair.second.clear();
//	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithoutTex) pair.second.clear();
//
//	// Process objects layer by layer for efficient visibility check
//	for (Layer* layer : layerManager.getAllLayers()) {
//		if (!layer || !layer->isVisible()) continue; // Skip non-visible layers (single flag check)
//
//		for (GameObject* obj : layer->getObjects()) {
//			if (!obj->hasComponent<Render>() || !obj->hasComponent<Transform>())
//				continue;
//			
//			Render* render = obj->getComponent<Render>();
//			Transform* transform = obj->getComponent<Transform>();
//
//			float scaleX = transform->flipX ? -transform->scaleX : transform->scaleX;
//
//			renderer::InstanceData data;
//			float angle = glm::radians(transform->rotation);
//			transform->mdlWorld = glm::mat4{ 1 };
//			transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
//			transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
//			transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(scaleX, transform->scaleY, transform->scaleZ));
//			data.model = transform->mdlWorld;
//			data.color = render->clr;
//			data.texParams = glm::vec4(0, 0, 1, 1);
//
//			// Animation handling (same as before)
//			if (obj->hasComponent<Animation>() && obj->hasComponent<StateMachine>())
//			{
//				Animation* animation = obj->getComponent<Animation>();
//				StateMachine* sm = obj->getComponent<StateMachine>();
//				AnimateState& as = animation->animState[static_cast<int>(sm->state)];
//					
//				if (render->hasAnimation && !as.texFile.empty()) {
//					glm::vec2 texOffSet{ as.currentFrameColumn / static_cast<float>(as.totalColumn), 1.f - ((as.currentFrameRow + 1) / static_cast<float>(as.totalRow)) };
//					glm::vec2 texScale{ 1.f / as.totalColumn, 1.f / as.totalRow };
//
//					// Only update animation if layer is updatable
//					if (as.play && layer->isUpdatable() && !EditorManager::isEditingMode() && !EditorManager::isPaused())
//					{
//						as.frameTimer += deltaTime;
//						if (as.frameTimer >= as.frameTime)
//						{
//							as.frameTimer -= as.frameTime;
//							if (as.currentFrameColumn >= as.lastFrame.x && as.currentFrameRow >= as.lastFrame.y)
//							{
//								if (as.loop) {
//									as.currentFrameColumn = (int)as.initialFrame.x;
//									as.currentFrameRow = (int)as.initialFrame.y;
//									as.frameTimer = 0;
//								}
//								else {
//									as.play = false;
//								}
//							}
//							else {
//								++as.currentFrameColumn;
//								if (as.currentFrameColumn >= as.totalColumn) {
//									as.currentFrameColumn = 0;
//									++as.currentFrameRow;
//									if (as.currentFrameRow > as.lastFrame.y) {
//										as.currentFrameColumn = (int)as.initialFrame.x;
//										as.currentFrameRow = (int)as.initialFrame.y;
//									}
//								}
//							}
//							animation->runItBack = true;
//						}
//					}
//					data.texParams = { texOffSet, texScale };
//					BatchKey key{ render->modelRef.shape, as.texHDL };
//					objectWithTex[key].push_back(data);
//				}
//				else {
//					BatchKey key{ render->modelRef.shape, 0 };
//					objectWithoutTex[key].push_back(data);
//				}
//			}
//			else if (render->hasTex) {
//				BatchKey key{ render->modelRef.shape, render->texHDL };
//				objectWithTex[key].push_back(data);
//			}
//			else {
//				BatchKey key{ render->modelRef.shape, 0 };
//				objectWithoutTex[key].push_back(data);
//			}
//		}
//	}
//}

void RenderSystem::instancePerBatch(std::map<BatchKey, std::vector<renderer::InstanceData>> &batch)
{
	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& objs : batch)
	{
		GLuint ssboID = objs.first.ssboID;
		if (objs.first.ssboID == 0)
		{
			glCreateBuffers(1, &objs.first.ssboID);
			ssboID = objs.first.ssboID;
			glNamedBufferStorage(
				ssboID,
				renderer::model::translationBufferSize, // Total byte size
				nullptr, // No initial data yet
				GL_DYNAMIC_STORAGE_BIT // Allows future updates
			);
			GLuint bindingIndex = 0;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssboID);
		}

		glNamedBufferSubData(
			ssboID,
			0, // Offset
			objs.second.size() * sizeof(renderer::InstanceData), // Total size
			objs.second.data() // Source data
		);
	}
}

void RenderSystem::clearBatches() {
	// Delete all SSBOs
	for (auto& [key, instances] : objectWithTex) {
		if (key.ssboID != 0) {
			glDeleteBuffers(1, &key.ssboID);
		}
	}
	for (auto& [key, instances] : objectWithoutTex) {
		if (key.ssboID != 0) {
			glDeleteBuffers(1, &key.ssboID);
		}
	}

	objectWithTex.clear();
	objectWithoutTex.clear();
}

void RenderSystem::fboAspectRatio(int& width, int& height) const {
	width = fboWidth;
	height = fboHeight;
}

void ParticleSystem::clearUsedID(size_t ID)
{
	for(size_t i = 0; i < Particle::UIDs.size(); ++i)
	{
		if (Particle::UIDs[i].uid == ID)
		{
			--Particle::UIDs[i].particleAmt;
			if (Particle::UIDs[i].particleAmt == 0)
			{
				Particle::UIDs.erase(Particle::UIDs.begin() + i);
				break;
			}
		}
	}
}

size_t ParticleSystem::IDGenerator()
{
	size_t ID = 0;
	bool empty = false;
	/*usedID.clear();
	if (Particle::UIDs.size() != 0)
		for(int i = 0; i < Particle::UIDs.size(); ++i)
			usedID.push_back(Particle::UIDs[i]);
	std::sort(usedID.begin(), usedID.end());*/
	std::sort(Particle::UIDs.begin(), Particle::UIDs.end(), [](const Particle::UID& a, const Particle::UID& b) {
		return a.uid < b.uid;
		});
	for (int i = 0; i < Particle::UIDs.size(); ++i)
	{
		++ID;
		if (ID != Particle::UIDs[i].uid)
		{
			empty = true;
			break;
		}
	}
	if (!empty)
		++ID;
	return ID;
}

Vector2D ParticleSystem::positionGenerator(float offset)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	float pi = std::numbers::pi_v<float>;
	float angle = dis(gen) * 2.0f * pi;

	Vector2D randomOffset;
	randomOffset.x = offset * cos(angle);
	randomOffset.y = offset * sin(angle);

	return randomOffset;
}

Vector2D ParticleSystem::velocityGenerator(Vector2D direction)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.001f, 1.0f);
	float speed = 3.f * dis(gen);

	//normalize direction
	float magnitute = Math_Sqrt(direction.x * direction.x + direction.y * direction.y);

	if (magnitute == 0.f)
		return Vector2D{ 0,0 };
	return Vector2D{ (direction.x / magnitute) * speed, (direction.y / magnitute) * speed };
}

void ParticleSystem::activateParticle(GameObjectManager& manager)
{
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Particle>())
		{
			Particle* particleData = object->getComponent<Particle>();
			Transform* masterTransform = object->getComponent<Transform>();
			if (particleData->master.isActive && particleData->isMaster)
			{
				//std::cout << "Particle Group being activated is: " << particleData->grpNo << std::endl;
				particleData->master.isActive = false;
				/*if (particleData->grpNo == 0)
					particleData->grpNo = IDGenerator(manager);*/
				//static int pid = 0;
				size_t UID = IDGenerator();
				Particle::UIDs.push_back(Particle::UID(UID, particleData->master.particleAmt));
				std::cout << "UID made is: " << UID << std::endl;
				for (size_t i = 0; i < particleData->master.particleAmt; ++i)
				{
					std::string name = "Particle_" + std::to_string(UID) + "_" + std::to_string(i);
					GameObject* newObj = manager.createGameObject(name);
					std::cout << "Created Particle Object: " << name << std::endl;

					Transform* transform = newObj->addComponent<Transform>();
					Render* render = newObj->addComponent<Render>();
					Particle* indivData = newObj->addComponent<Particle>();
					/*Physics* physics = */newObj->addComponent<Physics>();
					Vector2D spawnPos = positionGenerator(particleData->master.offset);
					transform->x = masterTransform->x + spawnPos.x;
					transform->y = masterTransform->y + spawnPos.y;
					transform->z = masterTransform->z;
					transform->scaleX = 0.5f;
					transform->scaleY = 0.5f;
					indivData->childData.velocity = velocityGenerator(spawnPos);
					indivData->childData.grpNo = UID;
					indivData->childData.timer = particleData->master.timer;
					render->clr = particleData->master.clr;
					indivData->isMaster = false;
				}
			}
		}
	}
}

void ParticleSystem::update(GameObjectManager& manager, float const& deltaTime)
{
	(void)deltaTime;
	activateParticle(manager);
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	//for testing purposes only
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Particle>())
		{
			Particle* particle = object->getComponent<Particle>();
			if (particle->isMaster)
			{
				if (InputHandler::isKeyTriggered(GLFW_KEY_P))
					particle->master.isActive = true;
			}
		}
	}
}
//void RenderSystem::renderCollision(Collision::AABB const& box, glm::vec3 const& clr)
//{
//	{
//		// Get corners from min and max
//		glm::vec2 min = box.getMin();
//		glm::vec2 max = box.getMax();
//
//		float vertices[] = {
//			min.x, min.y, 0.0f, // bottom left
//			max.x, min.y, 0.0f, // bottom right
//			max.x, max.y, 0.0f, // top right
//			min.x, max.y, 0.0f  // top left
//		};
//
//		GLuint VAO, VBO;
//		glGenVertexArrays(1, &VAO);
//		glGenBuffers(1, &VBO);
//
//		glBindVertexArray(VAO);
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//		glEnableVertexAttribArray(0);
//
//		// Use simple color shader (no texture)
//		glUseProgram(renderer::shdr_pgm[1]);
//
//		GLint vColorLoc = glGetUniformLocation(renderer::shdr_pgm[1], "uColor");
//		if (vColorLoc != -1)
//			glUniform3fv(vColorLoc, 1, glm::value_ptr(clr));
//
//		GLint vTransformView = glGetUniformLocation(renderer::shdr_pgm[1], "V");
//		GLint vTransformProj = glGetUniformLocation(renderer::shdr_pgm[1], "P");
//		glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(renderer::cam.view));
//		glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(renderer::cam.proj));
//
//		// Draw as line loop
//		glLineWidth(2.0f);
//		glDrawArrays(GL_LINE_LOOP, 0, 4);
//
//		glBindVertexArray(0);
//		glDeleteBuffers(1, &VBO);
//		glDeleteVertexArrays(1, &VAO);
//	}
//}

void FontSystem::init(GameObjectManager& manager)
{
	Font::init();

	ResourceManager::getInstance().getFont("assets/Orange Knight.ttf");
	ResourceManager::getInstance().getFont("assets/ARIAL.TTF");
	ResourceManager::getInstance().getFont("assets/times.ttf");

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<FontComponent>())
		{
			FontComponent* fc = object->getComponent<FontComponent>();
			fc->mdl = Font::fontMdls[0];
		}
	}
}

void FontSystem::update(GameObjectManager& manager, double fps)
{
	//for debugging only

	//std::cout << "=== FontSystem::update() called ===" << std::endl;
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	// Count font objects
	/*int fontCount = 0;
	for (GameObject* object : gameObjects) {
		if (object->hasComponent<FontComponent>()) {
			fontCount++;
			FontComponent* fc = object->getComponent<FontComponent>();
			std::cout << "Font object: " << object->getObjectName()
				<< " | Text: '" << fc->word << "'" << std::endl;
		}
	}*/
	//for debugging only
	//std::cout << "Total font objects: " << fontCount << std::endl;

	/*if (fontCount == 0) {
		std::cout << "WARNING: No font objects to render!" << std::endl;
		return;
	}*/

	glUseProgram(Font::fontShaders);

	glm::mat4 camView, camProj;
	if (UISystem::isShowUI()) {
		camView = renderer::editorCam.view;
		camProj = renderer::editorCam.proj;
	}
	else {
		camView = renderer::cam.view;
		camProj = renderer::cam.proj;
	}

	GLuint vTransformView = glGetUniformLocation(Font::fontShaders, "V");
	glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(camView));
	GLuint vTransformProj = glGetUniformLocation(Font::fontShaders, "P");
	glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(camProj));
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<FontComponent>())
		{
			FontComponent* fc = object->getComponent<FontComponent>();
			Transform* transform = object->getComponent<Transform>();
			RenderText(Font::fontShaders, fc->word, transform->x, transform->y, fc->scale, fc->clr, object);
		}
	}

	/* ---- scuffed way to render fps for M3 ---- */
	if (showFPS) {
		std::unique_ptr<GameObject> fpsText = manager.createTempGameObject("fpsText");
		if (Transform* t = fpsText->addComponent<Transform>()) {
			if (FontComponent* fc = fpsText->addComponent<FontComponent>()) {
				fc->word = "FPS: " + std::to_string(fps);
				t->x = -15.f;
				t->y = 9.f;
				RenderText(Font::fontShaders, fc->word, t->x, t->y, fc->scale, fc->clr, fpsText.get());
			}
		}
	}
	/* ---- END ---- */

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Font", ms }); //saving timing for UI output
}

void FontSystem::RenderText(GLuint& s, std::string text, float x, float y, float scale, glm::vec3 color, GameObject* object)
{
	// activate corresponding render state	
	FontComponent* fc = object->getComponent<FontComponent>();
	Font::FontMdl& mdl = fc->mdl;

	//Font::FontMdl& mdl = object->getComponent<FontComponent>()->mdl;
	//^mans trolling here
	
	std::string fontPath;
	switch (fc->fontType) {
	case 0: fontPath = "assets/Orange Knight.ttf"; break;
	case 1: fontPath = "assets/ARIAL.TTF"; break;
	case 2: fontPath = "assets/times.ttf"; break;
	default: fontPath = "assets/ARIAL.TTF"; break;
	}

	const FontData& fontData = ResourceManager::getInstance().getFont(fontPath);

	/*std::cout << "FontData has " << fontData.characters.size() << " characters loaded" << std::endl;
	if (fontData.characters.empty()) {
		std::cerr << "ERROR: Font data has NO characters!" << std::endl;
		return;
	}*/

	// --- Add this block --- //for text to be on top of everything
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// ----------------------

	glUseProgram(s);
	glUniform3f(glGetUniformLocation(s, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(mdl.vao);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		unsigned char uc = static_cast<unsigned char>(*c);
		auto it = fontData.characters.find(uc);
		//if (it == fontData.characters.end()) {
		//	std::cerr << "Character '" << uc << "' not found in font '" << fontPath << "'\n";
		//	continue; // skip character if not found
		//}

		const Font::Character& ch = it->second;
		//Font::Character ch = Font::characters[fc->fontType][*c];

		float pxToWorld = (2.0f * renderer::cam.zoom) / (renderer::cam.width / renderer::cam.ar);
		float xpos = x + (ch.Bearing.x * pxToWorld) * scale;
		float ypos = y - ((ch.Size.y - ch.Bearing.y) * pxToWorld) * scale;
		//float ypos = y - (ch.Bearing.y * pxToWorld) * scale;

		float w = (ch.Size.x * pxToWorld) * scale;
		float h = (ch.Size.y * pxToWorld) * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, mdl.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += ((ch.Advance >> 6) * pxToWorld) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- Restore state --- //for text to be on top of everything
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

// Physics system - updates position based on velocity and applies gravity
// uncomment this debug code to see all bullets and their states
//void debugBullets(GameObjectManager& manager) {
//	std::vector<GameObject*> gameObjects;
//	manager.getAllGameObjects(gameObjects);
//
//	for (GameObject* obj : gameObjects) {
//		if (obj->getObjectName().find("bullet") == 0) {
//			Physics* p = obj->getComponent<Physics>();
//			Transform* t = obj->getComponent<Transform>();
//			if (p && t) {
//				std::cout << "Bullet: " << obj->getObjectName()
//					<< " Alive: " << p->alive
//					<< " Pos: (" << t->x << ", " << t->y << ")"
//					<< " Vel: (" << p->dynamics.velocity.x << ", " << p->dynamics.velocity.y << ")\n";
//			}
//		}
//	}
//}

void PhysicsSystem::onEnterWater(Physics* physics, const float deltaTime) {
	(void) deltaTime;
	physics->hitWater = false; // reset the flag after handling water entry
	physics->inWater = true;
	physics->velX *= 0.5f; // Reduce horizontal speed
	physics->velY *= 0.5f; // Reduce vertical speed
	//physics->gravity = 9.8f * 0.5f; // Reduce gravity effect
}

void PhysicsSystem::update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus) {
	auto start = std::chrono::high_resolution_clock::now();

	// Step mode toggling (doesnt seem to be working with the new physics system)
	/*if (InputHandler::isComboKeyTriggered(GLFW_KEY_Z))
		m_stepMode = !m_stepMode;
	if (m_stepMode && InputHandler::isKeyTriggered(GLFW_KEY_Z))
		m_stepReq = true;
	if (m_stepMode && !m_stepReq)
		return;
	if (m_stepReq)
		m_stepReq = false;*/

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	std::unordered_map<GameObject*, bool> previousOnGround;
	for (auto object : gameObjects) {
		if (!object->hasComponent<Physics>()) continue;
		Physics* physics = object->getComponent<Physics>();
		previousOnGround[object] = physics->onGround;
		if(object->hasComponent<Particle>()) 
		{
			Particle* particle = object->getComponent<Particle>();
			if (!particle->isMaster)
			{
				Transform* transform = object->getComponent<Transform>();
				particle->childData.timer -= deltaTime;
				if (particle->childData.timer <= 0.f)
				{
					ParticleSystem::clearUsedID(static_cast<int>(particle->childData.grpNo));
					manager.deleteGameObject(object);
					continue;
				}
				transform->x += particle->childData.velocity.x * deltaTime;
				transform->y += particle->childData.velocity.y * deltaTime;
			}
		}
	}
	manager.getAllGameObjects(gameObjects);

	for (auto object : gameObjects) {
		if (!object->hasComponent<Physics>()) continue;
		Physics* physics = object->getComponent<Physics>();
		physics->onGround = false;
	}

	for (auto object : gameObjects) {
		if (!object->hasComponent<Transform>() || !object->hasComponent<Physics>())
			continue;

		Physics* physics = object->getComponent<Physics>();
		
		if (object->getObjectName() == "bullet") continue;
		
		Transform* transform = object->getComponent<Transform>();

		//sinle's personal physics for his one and only character, dont touch it
		if (physics->sinlePersonalBool)
		{
			//std::cout << "hregoesrbgiubgiubgbiguwbiub";
			physics->physicsFlag = false;
			if (physics->sinleFlag)
			{
				if (physics->inWater)
					physics->acceleration.y = physics->waterBounce;
				else
					physics->acceleration.y = 0;
				physics->velX += physics->acceleration.x * deltaTime;
				physics->velY += (physics->acceleration.y + physics->sinleGravity) * deltaTime;
				if(physics->hitWater)
					onEnterWater(physics, deltaTime);
				if (physics->hitground)
					physics->velY = 0;
				transform->x += physics->velX * deltaTime;
				transform->y += physics->velY * deltaTime;
				/*std::cout << "acceleration: (" << physics->acceleration.x << ", " << physics->acceleration.y << ")\n";
				std::cout << "velocity: (" << physics->velX << ", " << physics->velY << ")\n";
				std::cout << "position: (" << transform->x << ", " << transform->y << ")\n";*/
			}
			continue;
		}
		if (!physics->physicsFlag) continue;
		if (physics->autoUpdate) {
			// Apply gravity when not in water -- future ref
			if (!EditorManager::isEditingMode() && !EditorManager::isPaused()) {
				if (!physics->inWater) {
					physics->velY += physics->gravity * deltaTime;
				}

				transform->y += physics->velY * deltaTime;
				transform->x += physics->velX * deltaTime;

				physics->dynamics.position.y = transform->y;
				physics->dynamics.position.x = transform->x;
				physics->dynamics.velocity.y = physics->velY;
				physics->dynamics.velocity.x = physics->velX;
			}
		}

		// Handle player input
		if (object->hasComponent<Input>() && !EditorManager::isEditingMode() && !EditorManager::isPaused()) {
			// STEP 1: Ground check FIRST
			if (transform->y <= physics->floorY && !object->checkAutoMove()) {
				transform->y = physics->floorY;
				physics->dynamics.position.y = physics->floorY;
				physics->dynamics.velocity.y = 0.0f;
				physics->velY = 0.0f;
				physics->onGround = true;

				// coyote
				physics->coyoteAvailable = true; // reset when on ground
				physics->coyoteTimer = physics->coyoteTime;
			}

			// coyote timer update when not on ground
			if (!physics->onGround) {
				if (physics->coyoteTimer > 0.f)
					physics->coyoteTimer -= deltaTime;
				else
					physics->coyoteAvailable = false;
			}

			// jump buffer timer update
			if (physics->jumpBufferTimer > 0.f)
				physics->jumpBufferTimer -= deltaTime;

			if (physics->ledgeClimb) {
				float diff = physics->ledgeTargetY - transform->y;

				if (std::abs(diff) < 0.05f) {
					// reach top
					transform->y = physics->ledgeTargetY;
					physics->dynamics.position.y = physics->ledgeTargetY;
					physics->dynamics.velocity.y = 0.0f;
					physics->velY = 0.0f;
					physics->onGround = true;
					physics->ledgeClimb = false;
					physics->coyoteAvailable = true;
					physics->coyoteTimer = physics->coyoteTime;
				}
				else {
					// still climbing
					float step = physics->ledgeClimbSpeed * deltaTime;
					transform->y += std::min(step, diff);
					physics->dynamics.position.y = transform->y;
					physics->dynamics.velocity.y = 0.0f;
					physics->velY = 0.0f;
					physics->onGround = false;
				}
			}

			// STEP 2: Jump input
			// when space pressed, store the intention
			if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE))
				physics->jumpBufferTimer = physics->jumpBufferTime;

			// jump if buffer is active AND player is grounded or coyote available
			if (physics->jumpBufferTimer > 0.f && (previousOnGround[object] || physics->coyoteAvailable)) {
				PhysicsForces::jump(object);
				physics->jumpBufferTimer = 0.f;
				physics->coyoteAvailable = false;
				messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "SPACE", true }));
			}

			// tap for small jump, hold for big jump
			if (InputHandler::isKeyReleased(GLFW_KEY_SPACE)) {
				if (physics->dynamics.velocity.y > 0.0f)
					physics->dynamics.velocity.y *= 0.8f;
			}

			// STEP 3: Movement
			float targetVelX = 0.0f;
			if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
				physics->canMove = true;
				targetVelX = -physics->moveSpeed;
				transform->flipX = true;
				messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "A", true }));
			}
			else if (InputHandler::isKeyHeld(GLFW_KEY_D) && physics->canMove) {
				targetVelX = physics->moveSpeed;
				transform->flipX = false;
				messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "D", true }));
			}

			// Flip player to face mouse direction (only when no movement keys override)
			if (!InputHandler::isKeyHeld(GLFW_KEY_A) && !InputHandler::isKeyHeld(GLFW_KEY_D)) {
				Vector2D mouseWorldPos = InputHandler::getWorldMousePosition();
				transform->flipX = (mouseWorldPos.x < transform->x);
			}


			// STEP 4: Integrate physics
			if (!object->checkAutoMove() && !physics->ledgeClimb) {

				if (CollisionInfo* collision = object->getComponent<CollisionInfo>()) {
					PhysicsForces::applyDamping(physics, collision->material.friction, targetVelX, deltaTime);
					PhysicsForces::updatePosition(transform, physics, deltaTime);
				}
			}


			// Continuous shooting system
			if (physics->canShoot) {
				// Update shoot timer
				physics->shootTimer += deltaTime;

				// Check if mouse button is being held down
				if (InputHandler::isMouseLeftHeld()) {
#ifdef _DEBUG
					if ((!UISystem::isShowUI() || (SceneWindow::isSceneHovered() && !EditorManager::isEditingMode()))) {
#endif // _DEBUG
						if (VisualLC::globalFloatVar.count("bullets") &&
							VisualLC::globalFloatVar["bullets"] > 0) {
							if (physics->shootTimer >= physics->fireRate) {
								GameObject* bullet = PhysicsForces::findAvailableBullet(manager);

								if (bullet != nullptr) {
									// Get mouse position in world coordinates
									Vector2D mouseWorldPos;
#ifdef _DEBUG
									if (UISystem::isShowUI() && !EditorManager::isEditingMode()) {
										ImVec2 scenePos, sceneSize;
										EditorManager::getScenePosNSize(scenePos, sceneSize);
										mouseWorldPos = InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize);
									}
									else {
										mouseWorldPos = InputHandler::getWorldMousePosition();
									}
#else
									mouseWorldPos = InputHandler::getWorldMousePosition();
#endif

									Transform* spawnTransform = transform;
									{
										std::vector<GameObject*> allObjs;
										manager.getAllGameObjects(allObjs);
										for (GameObject* candidate : allObjs)
										{
											if (!candidate->hasComponent<WeaponComponent>()) continue;
											if (!candidate->hasComponent<Transform>()) continue;
											Transform* wt = candidate->getComponent<Transform>();
											if (wt->parentName == object->getObjectName())
											{
												spawnTransform = wt;
												//set a flay saying channel is pending to play sound
												if (bullet->hasComponent<AudioComponent>()) {
													AudioComponent* audio = bullet->getComponent<AudioComponent>();
													AudioChannel* ch = audio->getDefaultChannel();
													ch->isPendingPlay = true;

													// Shoot bullet toward mouse position
													PhysicsForces::shoot(bullet, spawnTransform, mouseWorldPos);
													messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "LEFT_CLICK", true }));


													VisualLogicSystem::broadcastBulletShoot();

													DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
													physics->shootTimer = 0.0f;
												}
												else {
													physics->shootTimer = physics->fireRate * 0.5f;
													DebugLog::addMessage("No bullets available!", DebugMode::PlaySimul);
												}
												break;
											}
										}
									}

									
								}
							}
						}
						else {
							DebugLog::addMessage("Out of bullets!", DebugMode::PlaySimul);
						}
#ifdef _DEBUG
					}
#endif // _DEBUG
				}
				//else if (InputHandler::isMouseLeftClicked()) { /// to shoot only one bullet at a time
				//	// Single shot mode: Only fire once on click (if autoFire is disabled)
				//	if (!physics->autoFire && physics->shootTimer >= physics->fireRate) {
				//		GameObject* bullet = PhysicsForces::findAvailableBullet(manager);

				//		if (bullet != nullptr) {
				//			Vector2D mouseWorldPos = InputHandler::getWorldMousePosition();
				//			PhysicsForces::shoot(bullet, transform, mouseWorldPos);
				//			messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "LEFT_CLICK", true }));

				//			if (bullet->hasComponent<AudioComponent>()) {
				//				AudioComponent* audio = bullet->getComponent<AudioComponent>();
				//				AudioChannel* ch = audio->getDefaultChannel();
				//				ch->isPendingPlay = true;
				//			}

				//			physics->shootTimer = 0.0f;
				//		}
				//	}
				//}
				else {
					physics->shootTimer = physics->fireRate;
				}
			}

			
		}

		// Auto-move objects
		if (object->checkAutoMove()) {
			transform->y += physics->dynamics.velocity.y * deltaTime;
			transform->x += physics->dynamics.velocity.x * deltaTime;
		}

		// Angular integration for rotatable objects (e.g. water wheel)
		if (!physics->freezeRotation) {
			DynamicsSystem::IntegrateAngular(physics->dynamics, deltaTime);
			transform->rotation += physics->dynamics.angularVelocity * deltaTime;
		}
	}

	// Bullet physics
	for (GameObject* obj : gameObjects) {
		std::string name = obj->getObjectName();
		// Skip if not a numbered bullet
		//if (name.find("bullet") != 0 || name.length() <= 6) continue;
		if (name.find("bullet") != 0) continue;

		bool isNumberedBullet = true;
		for (size_t i = 6; i < name.length(); i++) {
			if (!std::isdigit(name[i])) {
				isNumberedBullet = false;
				break;
			}
		}
		if (!isNumberedBullet) continue;

		if (!obj->hasComponent<Physics>() || !obj->hasComponent<Transform>())
			continue;

		Physics* p = obj->getComponent<Physics>();
		Transform* t = obj->getComponent<Transform>();

		if (p->alive) {
			// Update forces (non-gravity forces)
			DynamicsSystem::UpdateForces(p->dynamics, deltaTime);

			// Check gravity delay
			if (p->gravityDelayTimer > 0.0f) {
				// No gravity yet - just integrate velocity into position
				p->gravityDelayTimer -= deltaTime;

				Vector3D deltaPos = Vec3_Scale(&p->dynamics.velocity, deltaTime);
				p->dynamics.position = Vec3_Add(&p->dynamics.position, &deltaPos);
			}
			else {
				// Gravity delay expired - normal physics with gravity
				//DynamicsSystem::Integrate(p->dynamics, deltaTime, p->gravity, false);

				/*for water to move*/
				//flow force
				float flowDir = (p->dynamics.velocity.x >= 0.0f) ? 1.0f : -1.0f;
				float flowStrength = 25.0f * expf(-p->lifeTimer * 0.6f);
				p->dynamics.velocity.x += flowStrength * flowDir * deltaTime;
				p->dynamics.velocity.y += 0.0f;
				//strong gravity
				p->dynamics.velocity.y += -25.0f * deltaTime;
				//turbulence that decays over time
				float turbulence = sin(p->lifeTimer * 8.0f) * 2.0f * expf(-p->lifeTimer);
				p->dynamics.velocity.y += turbulence * deltaTime;
				//quadratic drag
				float speed = sqrt(
					p->dynamics.velocity.x * p->dynamics.velocity.x +
					p->dynamics.velocity.y * p->dynamics.velocity.y);

				if (speed > 0.0001f) {
					float dragCoefficient = 0.5f;
					float dragForce = dragCoefficient * speed * speed;
					p->dynamics.velocity.x -= (p->dynamics.velocity.x / speed) * dragForce * deltaTime;
					p->dynamics.velocity.y -= (p->dynamics.velocity.y / speed) * dragForce * deltaTime;
				}
				//integrate
				Vector3D deltaPos = Vec3_Scale(&p->dynamics.velocity, deltaTime);
				p->dynamics.position = Vec3_Add(&p->dynamics.position, &deltaPos);
			}

			t->x = p->dynamics.position.x;
			t->y = p->dynamics.position.y;

			// Update life timer
			p->lifeTimer += deltaTime;

			// Deactivate if lifetime exceeded or fell too far
			if (!EditorManager::isEditingMode()) {
				if (p->lifeTimer >= p->maxLifetime || p->dynamics.position.y < -20.0f) {
					PhysicsForces::deactivateBullet(obj);
				}
			}
		}
		else {
			//Haardcoded way-- need fix in future
			if (!EditorManager::isEditingMode()) {
				if (t->y > -100.0f || t->x > -100.0f) {
					t->x = -1000.0f;
					t->y = -1000.0f;
					p->dynamics.position.x = -1000.0f;
					p->dynamics.position.y = -1000.0f;
					p->dynamics.velocity.x = 0.0f;
					p->dynamics.velocity.y = 0.0f;
				}
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Physics", ms });
}

void WeaponSystem::update(GameObjectManager& manager, const float& deltaTime)
{
	auto start = std::chrono::high_resolution_clock::now();

	(void)deltaTime;

	#ifdef _DEBUG
		if (EditorManager::isEditingMode()) return;
	#endif

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	const Vector2D mouseWorld = InputHandler::getWorldMousePosition();

	for (GameObject* obj : gameObjects)
	{
		if (!obj->hasComponent<Transform>() || !obj->hasComponent<WeaponComponent>())
			continue;

		WeaponComponent* weapon = obj->getComponent<WeaponComponent>();
		if (!weapon->aimAtMouse) continue;

		Transform* weaponTransform = obj->getComponent<Transform>();


		if (!weapon->baseLocalOffsetInitialised)
		{
			weapon->baseLocalOffsetX = weaponTransform->localOffset.x;
			weapon->baseLocalOffsetInitialised = true;
		}

		bool ownerFacingLeft = false;

		if (!weaponTransform->parentName.empty())
		{
			GameObject* owner = manager.getGameObject(weaponTransform->parentName);
			if (owner && owner->hasComponent<Transform>())
			{
				ownerFacingLeft = owner->getComponent<Transform>()->flipX;
			}
		}

		weaponTransform->localOffset.x = ownerFacingLeft
			? -std::abs(weapon->baseLocalOffsetX)
			: std::abs(weapon->baseLocalOffsetX);

		if (!weaponTransform->parentName.empty())
		{
			GameObject* owner = manager.getGameObject(weaponTransform->parentName);
			if (owner && owner->hasComponent<Transform>())
			{
				Transform* pt = owner->getComponent<Transform>();
				const float parentRad = glm::radians(pt->rotation);
				const float cosA = cosf(parentRad);
				const float sinA = sinf(parentRad);
				const float rotatedX = weaponTransform->localOffset.x * cosA
					- weaponTransform->localOffset.y * sinA;
				const float rotatedY = weaponTransform->localOffset.x * sinA
					+ weaponTransform->localOffset.y * cosA;
				weaponTransform->x = pt->x + rotatedX;
				weaponTransform->y = pt->y + rotatedY;
			}
		}

		const float dx = mouseWorld.x - weaponTransform->x;
		const float dy = mouseWorld.y - weaponTransform->y;
		
		const float aimAngleRad = ownerFacingLeft
			? std::atan2(-dy, -dx)  
			: std::atan2(dy, dx); 
		weaponTransform->flipX = ownerFacingLeft;

		const float aimAngleDeg = glm::degrees(aimAngleRad);
		weaponTransform->rotation = aimAngleDeg;

		weapon->currentAngle = aimAngleDeg;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Weapon", ms });
}

//helper function for collision detection
static uint64_t MakePairKey(GameObject* a, GameObject* b)
{
	if (a > b) std::swap(a, b);   //enforce order
	return (uint64_t(a) << 32) | uint64_t(b); //combine into single key
}

// Collision system - detects and resolves collisions between game objects
void CollisionSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	double ms = 0.0;

	LayerManager& layerManager = manager.getLayerManager();
	std::vector<Layer*> layers = layerManager.getAllLayers();

	const float CELL_SIZE = 4.0f;
	std::unordered_map<int, Collision::Cell> grid;

	for (Layer* layer : layers) {
		if (!layer) continue;
		
		// Skip collision processing for non-updatable layers
		if (!layer->isUpdatable()) continue;

		std::unordered_set<uint64_t> checkedPairs;
		grid.clear();
		
		const std::vector<GameObject*>& layerObjects = layer->getObjects();

		std::vector<std::pair<TileMap*, Transform*>> activeTilemaps; //store tilemaps for later use
		for (GameObject* obj : layerObjects) {
			TileMap* tm = obj->getComponent<TileMap>();
			Transform* tmTransform = obj->getComponent<Transform>();

			if (!tm || !tmTransform) continue;

			if (tm->enableCol && tm->colChanged)
				Collision::buildTileCollisionBox(tm, tmTransform);

			if (!tm->collisionBoxs.empty()) // Only add if has collision boxes
				activeTilemaps.push_back({ tm, tmTransform });
		}

		//insert object into grid 
		for (GameObject* obj : layerObjects) {
			Transform* objT = obj->getComponent<Transform>();
			CollisionInfo* c = obj->getComponent<CollisionInfo>();
			if (!c || !objT || !c->collisionFlag) continue; //skip if object not suppose to collide, no transform and no collision

			int minCellX = 0, maxCellX = 0, minCellY = 0, maxCellY = 0;
			//square
			if (c->colliderType == shape::square){
				Collision::AABB box = Collision::getObjectAABBbyCollider(objT, c); // use custom collider instead of scale
				minCellX = static_cast<int>(std::floor(box.getMin().x / CELL_SIZE));
				maxCellX = static_cast<int>(std::floor(box.getMax().x / CELL_SIZE));
				minCellY = static_cast<int>(std::floor(box.getMin().y / CELL_SIZE));
				maxCellY = static_cast<int>(std::floor(box.getMax().y / CELL_SIZE));
			}
			//circle
			else if (c->colliderType == shape::circle) {
				//need one for circle
				Collision::Circle circle = Collision::getObjectCirclebyCollider(objT, c);

				float minX = circle.getCenter().x - circle.getRadius();
				float maxX = circle.getCenter().x + circle.getRadius();
				float minY = circle.getCenter().y - circle.getRadius();
				float maxY = circle.getCenter().y + circle.getRadius();

				minCellX = static_cast<int>(std::floor(minX / CELL_SIZE));
				maxCellX = static_cast<int>(std::floor(maxX / CELL_SIZE));
				minCellY = static_cast<int>(std::floor(minY / CELL_SIZE));
				maxCellY = static_cast<int>(std::floor(maxY / CELL_SIZE));
			}

			//putting the object into the grid cells it occupies
			for (int x = minCellX; x <= maxCellX; ++x) {
				for (int y = minCellY; y <= maxCellY; ++y) {
					int key = Collision::HashCell(x, y);
				grid[key].objects.push_back(obj);
				}
			}
		}

		//checking collision 
		//record current time for performance tracking
		auto start = std::chrono::high_resolution_clock::now();

		for (auto& [key, cell] : grid) {
			auto& cellObject = cell.objects; //get objects in the cell
			if (cellObject.size() < 2) continue; //skip if less than 2 objects because cant collide

			//loop through objects in the cell to check for collision
			for (size_t n1 = 0; n1 < cellObject.size(); n1++) {
				//object 1
				GameObject* obj1 = cellObject[n1];
				Transform* t1 = obj1->getComponent<Transform>();
				CollisionInfo* c1 = obj1->getComponent<CollisionInfo>();
				Physics* p1 = obj1->getComponent<Physics>();
				if (!p1 || !c1 || !t1) continue;

				Vector2D vel1{ p1->dynamics.velocity.x, p1->dynamics.velocity.y };
				//object 2 looping
				for (size_t m = n1 + 1; m < cellObject.size(); m++) {
					GameObject* obj2 = cellObject[m]; //get obj2

					//object 2
					Transform* t2 = obj2->getComponent<Transform>();
					CollisionInfo* c2 = obj2->getComponent<CollisionInfo>();
					Physics* p2 = obj2->getComponent<Physics>();
					if (!p2 || !t2 || !c2) continue;

					uint64_t pairKey = MakePairKey(obj1, obj2); //make unique key for the pair

					//skip if this pair was already checked in another cell
					if (checkedPairs.count(pairKey)) continue;
					checkedPairs.insert(pairKey);

					// Skip if these collision tags shouldn't collide
					if (!c1->shouldCollideWith(c2->collisionTag) ||
						!c2->shouldCollideWith(c1->collisionTag)) {
						continue;
					}

					Vector2D vel2{ p2->dynamics.velocity.x, p2->dynamics.velocity.y };

					//collision info
					CollisionInfo info;

					//if both square
					if (c1->colliderType == shape::square && c2->colliderType == shape::square) {
						Collision::AABB aabb1 = Collision::getObjectAABBbyCollider(t1, c1);
						Collision::AABB aabb2 = Collision::getObjectAABBbyCollider(t2, c2);
						info = Collision::CollisionIntersection_RectRect_Dynamic_Info(aabb1, vel1, aabb2, vel2);
					}
					//if both circle
					else if (c1->colliderType == shape::circle && c2->colliderType == shape::circle) {
						Collision::Circle circle1 = Collision::getObjectCirclebyCollider(t1, c1);
						Collision::Circle circle2 = Collision::getObjectCirclebyCollider(t2, c2);
						info = Collision::CollisionIntersection_CircleCircle_Dynamic_Info(circle1, vel1, circle2, vel2);
					}
					// if one is square one is circle
					else if (c1->colliderType == shape::square && c2->colliderType == shape::circle) {
						Collision::AABB aabb1 = Collision::getObjectAABBbyCollider(t1, c1);
						Collision::Circle circle2 = Collision::getObjectCirclebyCollider(t2, c2);
						info = Collision::CollisionIntersection_CircleAABB_Dynamic_Info(circle2, vel2, aabb1, vel1);
					}
					else if (c1->colliderType == shape::circle && c2->colliderType == shape::square) {
						Collision::AABB aabb2 = Collision::getObjectAABBbyCollider(t2, c2);
						Collision::Circle circle1 = Collision::getObjectCirclebyCollider(t1, c1);
						info = Collision::CollisionIntersection_CircleAABB_Dynamic_Info(circle1, vel1, aabb2, vel2);
					}
					else {
						continue; //current dont have other shapes of collision 
					}

					//checking if collided
					if (info.collided) {

						//water particle merge logic
						if (obj1->getComponent<CollisionInfo>()->collisionTag == "bullet" &&
							obj2->getComponent<CollisionInfo>()->collisionTag == "bullet"){
							if (!EditorManager::isEditingMode()) {
								if (Collision::HandleWaterParticleMerge(obj1, obj2)) continue; //skip normal collision
							}
						}

						// Visual logic events (if needed)
						if (obj1->hasComponent<VisualLC>()) {
							VisualLC* logic1 = obj1->getComponent<VisualLC>();
							logic1->curCollision.insert(obj2);
						}

						if (obj2->hasComponent<VisualLC>()) {
							VisualLC* logic2 = obj2->getComponent<VisualLC>();
							logic2->curCollision.insert(obj1);
						}

						if (c1->collisionRes == CollisionResponseMode::NoResponse || c2->collisionRes == CollisionResponseMode::NoResponse) continue;

						bool obj1IsStatic = (c1->collisionRes == CollisionResponseMode::StopWhenCollide);
						bool obj2IsStatic = (c2->collisionRes == CollisionResponseMode::StopWhenCollide);

						//helper lambda for bounce calculation
						auto ApplyBounce = [](Physics* p, CollisionInfo* c, const Vector2D& normal, bool isStatic) {
							//get current velocity
							Vector2D vel{ p->dynamics.velocity.x, p->dynamics.velocity.y };
							//velocity along collision normal
							float velAlongNormal = vel.x * normal.x + vel.y * normal.y;

							if (isStatic) {
								//static object
								float b = std::min(c->material.bounciness, 0.05f); //tiny bounce
								p->dynamics.velocity.x -= velAlongNormal * normal.x * (1.0f - b);
								p->dynamics.velocity.y -= velAlongNormal * normal.y * (1.0f - b);
							}
							else {
								//movable object
								float b = (c->material.bounciness > 0.0f) ? c->material.bounciness : 1.0f;
								p->dynamics.velocity.x = (p->dynamics.velocity.x - 2.0f * velAlongNormal * normal.x) * b;
								p->dynamics.velocity.y = (p->dynamics.velocity.y - 2.0f * velAlongNormal * normal.y) * b;
							}

							//clamp velocity to prevent explosion
							float maxVel = 5.0f; //tweak based on scale
							p->dynamics.velocity.x = std::clamp(p->dynamics.velocity.x, -maxVel, maxVel);
							p->dynamics.velocity.y = std::clamp(p->dynamics.velocity.y, -maxVel, maxVel);
							};

						// CASE 1: Both pushable
						if (!obj1IsStatic && !obj2IsStatic) {
							//push out of collision equally
							if (info.normal.x != 0) {
								t1->x += info.normal.x * info.penetration * 0.5f;
								t2->x -= info.normal.x * info.penetration * 0.5f;
								p1->dynamics.position.x = t1->x;
								p2->dynamics.position.x = t2->x;
							}
							//push out of collision equally
							if (info.normal.y != 0) {
								t1->y += info.normal.y * info.penetration * 0.5f;
								t2->y -= info.normal.y * info.penetration * 0.5f;
								p1->dynamics.position.y = t1->y;
								p2->dynamics.position.y = t2->y;
							}

							// Bounce or stop based on bounciness
							float combined = c1->material.combinedBounciness(c2->material);
							if (combined > 0.0f) {
								float dot1 = p1->dynamics.velocity.x * info.normal.x + p1->dynamics.velocity.y * info.normal.y;
								float dot2 = p2->dynamics.velocity.x * (-info.normal.x) + p2->dynamics.velocity.y * (-info.normal.y);

								p1->dynamics.velocity.x = (p1->dynamics.velocity.x - 2.0f * dot1 * info.normal.x) * combined;
								p1->dynamics.velocity.y = (p1->dynamics.velocity.y - 2.0f * dot1 * info.normal.y) * combined;
								p2->dynamics.velocity.x = (p2->dynamics.velocity.x - 2.0f * dot2 * (-info.normal.x)) * combined;
								p2->dynamics.velocity.y = (p2->dynamics.velocity.y - 2.0f * dot2 * (-info.normal.y)) * combined;
							}
							if (c1->material.bounciness > 0.0f || c2->material.bounciness > 0.0f) {
								//std::cout << "CASE1 bounce: " << obj1->getObjectName() << " vs " << obj2->getObjectName() << std::endl;
								ApplyBounce(p1, c1, info.normal, false);
								ApplyBounce(p2, c2, { -info.normal.x, -info.normal.y }, false);
							}
							else {
								// Original stop behavior
								if (info.normal.y != 0) {
									if (info.normal.y > 0) {
										// obj1 landed on top of obj2.
										// If obj2 is a floating platform in water, inject a
										// downward velocity impulse into it so it visibly dips
										// before the buoyancy system floats it back up.
										if (c2->collisionTag == "floatingObject" &&
											p1->velY < -1.0f)
										{
											float impactFraction = 0.7f; // tune: higher = deeper dip
											float mass1 = p1->dynamics.mass;
											float mass2 = p2->dynamics.mass;
											float massRatio = glm::clamp(mass1 / mass2, 0.1f, 3.0f);
											p2->velY += p1->velY * impactFraction * massRatio;
											p2->velY = glm::max(p2->velY, -15.0f);
											p2->dynamics.velocity.y = p2->velY;
										}
										p1->dynamics.velocity.y = 0.f;
										p1->velY = 0.f;
										p1->onGround = true;

										p1->coyoteAvailable = true;
										p1->coyoteTimer = p1->coyoteTime;
									}
									else {
										// obj2 landed on top of obj1 — mirror logic
										if (c1->collisionTag == "floatingObject" &&
											p2->velY < -1.0f)
										{
											float impactFraction = 0.35f;
											float mass2val = p2->dynamics.mass;
											float mass1val = p1->dynamics.mass;
											float massRatio = glm::clamp(mass2val / mass1val, 0.1f, 3.0f);
											p1->velY += p2->velY * impactFraction * massRatio;
											p1->velY = glm::max(p1->velY, -15.0f);
											p1->dynamics.velocity.y = p1->velY;
										}
										p2->dynamics.velocity.y = 0.f;
										p2->velY = 0.f;
										p2->onGround = true;

										p2->coyoteAvailable = true;
										p2->coyoteTimer = p2->coyoteTime;
									}
								}
							}
						}
						// CASE 2: obj1 is pushable, obj2 is static
						else if (!obj1IsStatic && obj2IsStatic) {
							float pushPercent = 0.3f;
							// Push out of collision
							if (info.normal.x != 0) {
								t1->x += info.normal.x * info.penetration * pushPercent;

								// ledge assistance
								if (p1->dynamics.velocity.y > 0.0f) {
									Collision::AABB playerAABB = Collision::getObjectAABBbyCollider(t1, c1);
									Collision::AABB platAABB = Collision::getObjectAABBbyCollider(t2, c2);

									float distToTop = platAABB.getMax().y - playerAABB.getMin().y;

									if (distToTop > 0.f && distToTop < p1->ledgeAssistRange) {
										p1->ledgeClimb = true;
										p1->ledgeTargetY = t1->y + distToTop;
										continue;
									}
								}

								//stop horizontal motion for non-bullet objects
								if (c1->collisionTag != "bullet") { 
									p1->dynamics.velocity.x = 0.f;
									p1->velX = 0.f;
								}
							}
							if (info.normal.y != 0) {
								t1->y += info.normal.y * info.penetration * pushPercent;
								p1->dynamics.position.y = t1->y;

								if (c1->collisionTag == "bullet") { // water
									float spread = 10.f; // horizontal spread for flow
									float flowSpeed = 0.2f; // constant lateral push
									float direction = (rand() % 2 == 0 ? 1.f : -1.f);

									//p1->dynamics.velocity.x += flowSpeed * deltaTime + spread * direction;
									//p1->dynamics.velocity.y += 0.f;

									p1->dynamics.velocity.x += flowSpeed + spread * direction;
									
									// small downward bias so it stays on ground but can slide
									p1->dynamics.velocity.y = -0.01f; // tiny downward motion

									// apply sliding friction
									float slideFriction = -0.10f;
									p1->dynamics.velocity.x *= (1.0f - slideFriction);

									// keep onGround true for other logic if needed
									p1->onGround = true;
								}
								else {
									//normal objects
									p1->dynamics.velocity.y = 0.f;
								}
							}
							// Apply torque to obj2 if it can rotate (e.g. water wheel hit by bullet)
							// If obj2 is a child, redirect torque to its parent (e.g. spoke → water wheel)
							Physics* torqueTarget = p2;
							Transform* torqueOrigin = t2;
							if (!t2->parentName.empty()) {
								GameObject* par = manager.getGameObject(t2->parentName);
								if (par && par->hasComponent<Physics>() && par->hasComponent<Transform>()) {
									torqueTarget = par->getComponent<Physics>();
									torqueOrigin = par->getComponent<Transform>(); // lever arm from wheel centre
								}
							}
							if (!torqueTarget->freezeRotation && torqueTarget->dynamics.inertia > 0.0f) {
								Vector2D r = { info.contactPoint.x - torqueOrigin->x, info.contactPoint.y - torqueOrigin->y };
								float impulseMagnitude = p1->dynamics.mass *
									(r.x * p1->dynamics.velocity.y - r.y * p1->dynamics.velocity.x);
								torqueTarget->dynamics.angularVelocity += impulseMagnitude / torqueTarget->dynamics.inertia;
							}
							// Bounce or stop based on bounciness
							float combinedBounciness = c1->material.combinedBounciness(c2->material);
							if (combinedBounciness > 0.0f) {
								float dot = p1->dynamics.velocity.x * info.normal.x +
									p1->dynamics.velocity.y * info.normal.y;
								p1->dynamics.velocity.x = (p1->dynamics.velocity.x - 2.0f * dot * info.normal.x) * combinedBounciness;
								p1->dynamics.velocity.y = (p1->dynamics.velocity.y - 2.0f * dot * info.normal.y) * combinedBounciness;
							}

							if (c1->material.bounciness > 0.0f) {
								//float b = std::min(p1->bounciness, 0.05f); //tiny bounce
								//std::cout << "CASE2 bounce: " << obj1->getObjectName() << " vs " << obj2->getObjectName() << std::endl;
								ApplyBounce(p1, c1, info.normal, true);
							}
							else {
								//original stop behaviors
								if (c1->collisionTag != "bullet") {
									if (info.normal.x != 0) {
										p1->dynamics.velocity.x = 0.f;
										p1->velX = 0.f;
									}
									if (info.normal.y != 0) {
										p1->dynamics.velocity.y = 0.f;
										if (info.normal.y > 0) {
											p1->onGround = true;
											p1->coyoteAvailable = true;
											p1->coyoteTimer = p1->coyoteTime;
										}
									}
								}
							}
						}
						// CASE 3: obj1 is static, obj2 is pushable
						else if (obj1IsStatic && !obj2IsStatic) {
							// Push out of collision
							if (info.normal.x != 0) {
								if (p2->dynamics.velocity.y > 0.0f) {
									Collision::AABB playerAABB = Collision::getObjectAABBbyCollider(t2, c2);
									Collision::AABB platAABB = Collision::getObjectAABBbyCollider(t1, c1);

									float distToTop = platAABB.getMax().y - playerAABB.getMin().y;

									if (distToTop > 0.0f && distToTop < p2->ledgeAssistRange) {
										/*t2->y += distToTop;
										p2->dynamics.position.y = t2->y;
										p2->dynamics.velocity.y = 0.0f;
										p2->velY = 0.0f;
										p2->onGround = true;
										p2->coyoteAvailable = true;
										p2->coyoteTimer = p2->coyoteTime;
										return;*/
										p2->ledgeClimb = true;
										p2->ledgeTargetY = t2->y + distToTop;

										continue;
									}
								}

								t2->x -= info.normal.x * info.penetration;

								if (c2->collisionTag == "bullet") {
									// FLOWING BULLET: slide horizontally
									float spread = 10.f;
									float flowSpeed = 20.f;
									float direction = (rand() % 2 == 0 ? 1.f : -1.f);

									p2->dynamics.velocity.x += flowSpeed * deltaTime + spread * direction;
									p2->dynamics.velocity.y += 0.f; // optional slight downward movement
								}
								else {
									// normal object stops horizontally
									p2->dynamics.velocity.x = 0.f;
									p2->velX = 0.f;
								}
								p2->dynamics.position.x = t2->x;
							}

							if (info.normal.y != 0) {
								t2->y -= info.normal.y * info.penetration;
								p2->dynamics.position.y = t2->y;

								if (c2->collisionTag != "bullet") {
									p2->dynamics.velocity.y = 0.f;
									if (info.normal.y < 0) {
										p2->onGround = true;
										p2->coyoteAvailable = true;
										p2->coyoteTimer = p2->coyoteTime;
									}
								}
							}

							// Apply torque to obj1 if it can rotate
							// If obj1 is a child, redirect torque to its parent
							Physics* torqueTarget = p1;
							Transform* torqueOrigin = t1;
							if (!t1->parentName.empty()) {
								GameObject* par = manager.getGameObject(t1->parentName);
								if (par && par->hasComponent<Physics>() && par->hasComponent<Transform>()) {
									torqueTarget = par->getComponent<Physics>();
									torqueOrigin = par->getComponent<Transform>();
								}
							}
							if (!torqueTarget->freezeRotation && torqueTarget->dynamics.inertia > 0.0f) {
								Vector2D r = { info.contactPoint.x - torqueOrigin->x, info.contactPoint.y - torqueOrigin->y };
								float impulseMagnitude = p2->dynamics.mass *
									(r.x * p2->dynamics.velocity.y - r.y * p2->dynamics.velocity.x);
								torqueTarget->dynamics.angularVelocity += impulseMagnitude / torqueTarget->dynamics.inertia;
							}
							// Bounce or stop based on bounciness
							float combinedBounciness = c1->material.combinedBounciness(c2->material);
							if (combinedBounciness > 0.0f) {
								Vector2D n = { -info.normal.x, -info.normal.y }; // flip: normal is from p1's perspective
								float dot = p2->dynamics.velocity.x * n.x + p2->dynamics.velocity.y * n.y;
								p2->dynamics.velocity.x = (p2->dynamics.velocity.x - 2.0f * dot * n.x) * combinedBounciness;
								p2->dynamics.velocity.y = (p2->dynamics.velocity.y - 2.0f * dot * n.y) * combinedBounciness;
							}
							if (c2->material.bounciness > 0.0f) {
								//std::cout << "CASE3 bounce: " << obj1->getObjectName() << " vs " << obj2->getObjectName() << std::endl;
								ApplyBounce(p2, c2, { -info.normal.x, -info.normal.y }, true);
							}
							else {
								// Original stop behavior (only when NOT bouncing)
								if (info.normal.x != 0) {
									p2->dynamics.velocity.x = 0.f;
									p2->velX = 0.f;
								}
								if (info.normal.y != 0) {
									p2->dynamics.velocity.y = 0.f;
									if (info.normal.y < 0) {
										p2->onGround = true;
										p2->coyoteAvailable = true;
										p2->coyoteTimer = p2->coyoteTime;
									}
								}
							}
						}
					}
				}
			}
		}
		// obj vs tilemap collision
		if (!activeTilemaps.empty()) HandleTilemapCollisions(grid, activeTilemaps);

		auto end = std::chrono::high_resolution_clock::now();
		ms += std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	}
	g_SystemTimers.push_back({ "Collisions", ms }); //saving timing for UI output
}

void CollisionSystem::HandleTilemapCollisions(const std::unordered_map<int, Collision::Cell>& grid, const std::vector<std::pair<TileMap*, Transform*>>& tilemaps) {
	for (const auto& [key, cell] : grid) {
		for (GameObject* obj : cell.objects) {
			Transform* t = obj->getComponent<Transform>();
			CollisionInfo* c = obj->getComponent<CollisionInfo>();
			Physics* p = obj->getComponent<Physics>();

			if (!t || !c || !p) continue;

			Collision::AABB objAABB = Collision::getObjectAABBbyCollider(t, c);
			Vector2D vel{ p->dynamics.velocity.x, p->dynamics.velocity.y };

			// check against all active tilemaps
			for (const auto& [tm, tmtransform] : tilemaps) {
				bool collisionResolved = false;

				for (const auto& box : tm->collisionBoxs) {
					Collision::AABB boxAABB(box.minAABB, box.maxAABB);

					if (!Collision::CollisionIntersection_RectRect_Static(objAABB, boxAABB))
						continue;

					CollisionInfo info = Collision::CollisionIntersection_RectRect_Dynamic_Info(
						objAABB, vel, boxAABB, Vector2D(0, 0));

					if (info.collided) {
						// Push out of collision first
						if (info.normal.x != 0) {
							t->x += info.normal.x * info.penetration;
							p->dynamics.position.x = t->x;
						}
						if (info.normal.y != 0) {
							t->y += info.normal.y * info.penetration;
							p->dynamics.position.y = t->y;
						}

						// Bounce or stop based on bounciness
						float bounce = c->material.bounciness; // tile has no material, treat as 0 → average = b/2, but bullet hits tile directly so use full value
						if (bounce > 0.0f) {
							float dot = p->dynamics.velocity.x * info.normal.x + p->dynamics.velocity.y * info.normal.y;
							p->dynamics.velocity.x = (p->dynamics.velocity.x - 2.0f * dot * info.normal.x) * bounce;
							p->dynamics.velocity.y = (p->dynamics.velocity.y - 2.0f * dot * info.normal.y) * bounce;
						}
						else {
							// Normal stop response (player, etc.)
							if (info.normal.x != 0) {
								if (p->dynamics.velocity.y > 0.f) {
									Collision::AABB playerAABB = Collision::getObjectAABBbyCollider(t, c);
									float distToTop = boxAABB.getMax().y - playerAABB.getMin().y;

									if (distToTop > 0.f && distToTop < p->ledgeAssistRange) {
										p->ledgeClimb = true;
										p->ledgeTargetY = t->y + distToTop;
										collisionResolved = true;

										break;
									}
								}

								t->x += info.normal.x * info.penetration;
								p->dynamics.position.x = t->x;
								p->dynamics.velocity.x = 0.f;
								p->velX = 0.f;
							}
							if (info.normal.y != 0) {
								p->dynamics.velocity.y = 0.0f;
								if (info.normal.y > 0) {
									p->onGround = true;
									p->coyoteAvailable = true;
									p->coyoteTimer = p->coyoteTime;
								}
							}
						}

						collisionResolved = true;
						break;
					}
				}

				if (collisionResolved) break;
			}
		}
	}
}


#ifdef _DEBUG
//UI system - handles ImGui rendering and interaction
bool UISystem::showUI = false;
ImFont* UISystem::m_fontDefault = nullptr;
ImFont* UISystem::m_fontArial = nullptr;
ImFont* UISystem::m_fontTimes = nullptr;
UISystem::FontType UISystem::m_currentFontType = FONT_ARIAL;

void UISystem::init(GLFWwindow* window, RenderSystem* renderer, GameObjectManager& manager) {
	// --- ImGui Setup ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;

	// font settings
	m_fontArial = io.Fonts->AddFontFromFileTTF("assets/ARIAL.ttf", 25.0f);
	m_fontTimes = io.Fonts->AddFontFromFileTTF("assets/times.ttf", 25.0f);

	// fallback
	m_fontDefault = io.Fonts->AddFontDefault();

	io.FontDefault = m_fontDefault;


	io.FontGlobalScale = 1.5f;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsClassic();
	//ImGui::StyleColorsDark();
	// io.IniFilename = "imgui_layout.ini";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	// no show task bar icon for additional viewports
	io.ConfigViewportsNoTaskBarIcon = true;

	m_renderer = renderer;

	m_UI = std::make_unique<EditorManager>(manager);
}

void UISystem::setFont(FontType type) {
	ImGuiIO& io = ImGui::GetIO();

	switch (type) {
	case FONT_ARIAL:
		if (m_fontArial) {
			io.FontDefault = m_fontArial;
			m_currentFontType = FONT_ARIAL;
		}
		break;
	case FONT_TIMES:
		if (m_fontTimes) {
			io.FontDefault = m_fontTimes;
			m_currentFontType = FONT_TIMES;
		}
		break;
	case FONT_DEFAULT:
		if (m_fontDefault) {
			io.FontDefault = m_fontDefault;
			m_currentFontType = FONT_DEFAULT;
		}
		break;
	}
}

UISystem::~UISystem() {
	// cleanup all imgui stuff here
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//for imgui
void UISystem::update() {
	if (!showUI) return; // skip UI rendering if disabled

	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

	/*std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);*/

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	dockingSetUp();

	// get the width and height of the SCENE view
	int fboWidth, fboHeight;
	m_renderer->fboAspectRatio(fboWidth, fboHeight);

	// all ImGui windows
	m_UI->update();
	m_UI->renderScene(m_renderer->getTexture(), static_cast<float>(fboWidth) / static_cast<float>(fboHeight));

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	renderMultipleViewPorts();

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({"IMGUI", ms }); //saving timing for UI output
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
		| ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar(2);

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspaceID, { 0.0f, 0.0f }, dockspace_flags);

	if (m_firstTime) {
		m_firstTime = false;
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

		ImGuiID dockMain = dockspaceID;

		ImGuiID dockLeft, dockRight, dockBottom;
		ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.28f, &dockLeft, &dockMain);
		// split off right panel (30% of remaining width)
		ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.35f, &dockRight, &dockMain);
		// split off bottom panel (30% of remaining height)
		ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.50f, &dockBottom, &dockMain);

		// subdivide the panels
		ImGuiID dockLeftBottom, dockRightBottom;
		// split left panel into top and bottom
		ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.40f, &dockLeftBottom, &dockLeft);
			// split right panel into top and bottom
		ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.40f, &dockRightBottom, &dockRight);


		// dock windows to their respective areas
		// left top: Hierarchy
		ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);

		// left bottom: Debug
		ImGui::DockBuilderDockWindow("Debug - Editor", dockLeftBottom);
		ImGui::DockBuilderDockWindow("Debug - Playing", dockLeftBottom);

		// center: Scene viewport
		ImGui::DockBuilderDockWindow("Scene", dockMain);
		ImGui::DockBuilderDockWindow("Visual Logic Editor", dockMain);

		// right top: Inspector
		ImGui::DockBuilderDockWindow("Inspector", dockRight);
		ImGui::DockBuilderDockWindow("Editor Camera", dockRight);

		// right bottom: Create Object
		ImGui::DockBuilderDockWindow("Create Object", dockRightBottom);

		// bottom: Assets and Performance
		ImGui::DockBuilderDockWindow("Assets", dockBottom);
		ImGui::DockBuilderDockWindow("Performance", dockBottom);

		ImGui::DockBuilderFinish(dockspaceID);
	}

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
#endif

//void LogicSystem::update(GameObjectManager& manager, float const& dt) {
//
//	std::vector<GameObject*> objs;
//	manager.getAllGameObjects(objs);
//
//	static LogicContainer container; // stateless helper
//
//	for (auto* go : objs) {
//		if (!go) continue;
//		if (!go->hasComponent<StateMachine>()) continue;
//		if (!go->hasComponent<Transform>() || !go->hasComponent<Physics>()) continue;
//
//		container.update(*go, dt /*, manager */);
//	}
//}

void LogicSystem::update(GameObjectManager& manager, float const& dt) {
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> objs;
	manager.getAllGameObjects(objs);

	static LogicContainer container;
	
	// DEBUG: Run once to show what objects exist
	static bool debugOnce = true;
	if (debugOnce) {
		debugOnce = false;
		std::cout << "\n[LogicSystem DEBUG] ====== OBJECT FSM ASSIGNMENT ======" << std::endl;
		for (auto* go : objs) {
			if (!go) continue;
			CollisionInfo* col = go->getComponent<CollisionInfo>();
			std::string tag = col ? col->collisionTag : "(no CollisionInfo)";
			bool hasSM = go->hasComponent<StateMachine>();
			bool hasInput = go->hasComponent<Input>();
			
		std::cout << "[DEBUG] Object: " << go->getObjectName() 
		          << " | Layer: " << go->getLayer()
		          << " | Tag: " << tag
			          << " | StateMachine: " << (hasSM ? "YES" : "NO")
			          << " | Input: " << (hasInput ? "YES" : "NO")
			          << " | Physics: " << (go->hasComponent<Physics>() ? "YES" : "NO")
			          << " | Animation: " << (go->hasComponent<Animation>() ? "YES" : "NO")
			          << "\n";
			
			// Check if this object SHOULD have an FSM
			if (col && col->collisionTag == "player") {
				std::cout << "       -> Player object detected!\n";
				if (!hasSM) std::cout << "       -> WARNING: Missing StateMachine component!\n";
				if (!hasInput) std::cout << "       -> WARNING: Missing Input component!\n";
			}
		}
		std::cout << "[LogicSystem DEBUG] ======================================\n" << std::endl;
	}

	for (auto* go : objs) {
		if (!go) continue;

		// Get collision tag to determine which FSM to use
		CollisionInfo* col = go->getComponent<CollisionInfo>();
		std::string tag = col ? col->collisionTag : "";

		// Player FSM
		if (go->hasComponent<StateMachine>() && go->hasComponent<Input>()) {
			if (!go->getComponent<StateMachine>()->fixState) {
				container.update(*go, dt);
			}
		}
		// Fire FSM
		else if (tag == "fire" && go->hasComponent<StateMachine>()) {
			container.updateFire(*go, dt);
		}
		// Water Refill FSM
		else if (tag == "water" && go->hasComponent<StateMachine>()) {
			// DEBUG: Verify this is being called
			static bool waterDebugOnce = true;
			if (waterDebugOnce) {
				std::cout << "[DEBUG] updateRefill() being called for: " << go->getObjectName() << std::endl;
				waterDebugOnce = false;
			}
			container.updateRefill(*go, dt);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Logic", ms });
}

/*
This function runs once when scene load and system starts
*/
void AudioSystem::init(GameObjectManager& manager) {
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	AudioHandler& audioHandler = AudioHandler::getInstance();

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;

		AudioComponent* audio = obj->getComponent<AudioComponent>();

		// Process ALL channels with playOnStart=true
		for (auto& [channelName, ch] : audio->audioChannels) {
			if (ch.playOnStart) {
				ch.channel = audioHandler.playSound(&ch);
				if (ch.channel) {
					ch.state = AudioState::Playing;

					// Apply fade in
					if (ch.fadeInOnStart) {
						audioHandler.fadeIn(&ch, ch.volume, ch.fadeInDuration);
					}

					// Apply pitch
					if (ch.pitch != 1.0f) {
						audioHandler.setSoundpitch(&ch, ch.pitch);
					}

					// Apply mute state
					if (ch.muted) {
						audioHandler.muteSound(&ch);
					}
				}
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Audio Init", ms });
}

void AudioSystem::update(GameObjectManager& manager, float deltaTime) {
	auto start = std::chrono::high_resolution_clock::now();

	// Update FMOD listener position
	GameObject* player = manager.getGameObject("player");
	if (player && player->hasComponent<Transform>()) {
		Transform* playerTransform = player->getComponent<Transform>();
		AudioHandler::getInstance().updateListener(
			playerTransform->x,
			playerTransform->y,
			playerTransform->z,
			0.0f, 0.0f, 1.0f,  // forward: looking into screen
			0.0f, 1.0f, 0.0f   // up: Y-axis is up
		);
	}

	// Update all sound source positions
	updateSpatialAudio(manager);

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	AudioHandler& audioHandler = AudioHandler::getInstance();

	for (GameObject* obj : gameObjects) {
		// CRITICAL CHECK #1: Verify GameObject pointer is valid
		if (!obj) {
			std::cerr << "[AudioSystem] Null GameObject encountered in update loop!\n";
			continue;
		}

		// CRITICAL CHECK #2: Verify GameObject has AudioComponent before accessing
		if (!obj->hasComponent<AudioComponent>()) continue;

		// CRITICAL CHECK #3: Verify AudioComponent pointer is valid
		AudioComponent* audio = obj->getComponent<AudioComponent>();
		if (!audio) {
			std::cerr << "[AudioSystem] getComponent<AudioComponent>() returned nullptr for object: "
				<< obj->getObjectName() << std::endl;
			continue;
		}

		// CRITICAL CHECK #4: Verify audioChannels map is accessible
		// (Attempt to access the map - if audio pointer is invalid, this will crash)
		try {
			// Safe to call updateFades now
			audio->updateFades(deltaTime);

			for (auto& pair : audio->audioChannels) {
				AudioChannel& ch = pair.second;

				// Background music restart logic
				if (ch.playOnStart && ch.loop) {
					if (!ch.channel || !audioHandler.isSoundPlaying(&ch)) {
						// Restart background music if not playing
						ch.channel = audioHandler.playSound(&ch);
						if (ch.channel) {
							ch.state = AudioState::Playing;
							audioHandler.setSoundpitch(&ch, ch.pitch);
							if (ch.muted) {
								audioHandler.muteSound(&ch);
							}
						}
					}
				}

				// Process audio (isPendingPlay, isPendingStop, etc.)
				processAudio(obj, &ch, deltaTime);

				// Check if channel is still playing
				if (ch.channel) {
					bool isPlaying = false;
					ch.channel->isPlaying(&isPlaying);
					if (!isPlaying) {
						ch.channel = nullptr;
						ch.fadeInfo.isFading = false;
						ch.state = AudioState::Stopped;
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "[AudioSystem] Exception processing audio for object: "
				<< obj->getObjectName() << " - " << e.what() << std::endl;
			continue;
		}
		catch (...) {
			std::cerr << "[AudioSystem] Unknown exception processing audio for object: "
				<< obj->getObjectName() << std::endl;
			continue;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Audio", ms });
}

/*
 * ADDITIONAL FIX: Add null check in processAudio
 */
void AudioSystem::processAudio(GameObject* obj, AudioChannel* audio, float deltaTime) {
	(void)deltaTime;
	(void)obj;
	AudioHandler& audioHandler = AudioHandler::getInstance();


	// CRITICAL CHECK: Verify audio pointer is valid
	if (!audio) {
		std::cerr << "[AudioSystem::processAudio] Null AudioChannel pointer!\n";
		return;
	}

	//AudioHandler& audioHandler = AudioHandler::getInstance();

	if (audio->isPendingStop) {
		audio->isPendingStop = false;

		if (audio->fadeOutOnStop && audio->state == AudioState::Playing) {
			audioHandler.fadeOut(audio, audio->fadeOutDuration);
		}
		else {
			audioHandler.stopSound(audio);
			audio->channel = nullptr;
			audio->state = AudioState::Stopped;
		}
	}

	if (audio->isPendingPlay) {
		audio->isPendingPlay = false;

		// CRITICAL CHECK: Verify audioFile is set
		if (audio->audioFile.empty()) {
			std::cerr << "[AudioSystem] Cannot play audio: audioFile is empty for object: "
				<< (obj ? obj->getObjectName() : "unknown") << std::endl;
			audio->state = AudioState::Stopped;
			return;
		}

		if (audio->channel != nullptr) {
			audioHandler.stopSound(audio);
			audio->channel = nullptr;
		}
		audio->state = AudioState::Stopped;

		audio->channel = audioHandler.playSound(audio);
		if (audio->channel) {
			audio->state = AudioState::Playing;

			if (audio->fadeInOnStart) {
				audioHandler.fadeIn(audio, audio->volume, audio->fadeInDuration);
			}

			if (audio->pitch != 1.0f) {
				audioHandler.setSoundpitch(audio, audio->pitch);
			}

			if (audio->muted) {
				audioHandler.muteSound(audio);
			}
		}
		else {
			audio->state = AudioState::Stopped;
		}
	}

	// Update state 
	if (audio->channel && audio->state == AudioState::Playing) {
		if (!audioHandler.isSoundPlaying(audio)) {
			audio->state = AudioState::Stopped;
			audio->channel = nullptr;
		}
	}

	if (audio->state == AudioState::Playing && audio->channel) {
		// Check if paused 
		if (audioHandler.isSoundPaused(audio)) {
			audio->state = AudioState::Paused;
		}
	}
}

void AudioSystem::cleanup(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	AudioHandler& audioHandler = AudioHandler::getInstance();
	
	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;

		AudioComponent* audio = obj->getComponent<AudioComponent>();

		for (auto& pair : audio->audioChannels) {
			AudioChannel& ch = pair.second;

			if (ch.state == AudioState::Playing || ch.state == AudioState::Paused) {
				audioHandler.stopSound(&ch);
				ch.channel = nullptr;
				ch.state = AudioState::Stopped;
			}
		}

	}
}

void AudioSystem::initializeSceneAudio(GameObjectManager& manager) {
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;

		AudioComponent* audio = obj->getComponent<AudioComponent>();

		// Pre-load all sounds for all channels
		for (auto& [channelName, channel] : audio->audioChannels) {
			if (!channel.audioFile.empty()) {
				// Pass isSpatial flag during preload, true source of the sound
				FMOD::Sound* sound = ResourceManager::getInstance().getSound(
					channel.audioFile,
					channel.loop,
					channel.isSpatial
				);

				if (!sound) {
					std::cerr << "[AudioSystem] Failed to preload: "
						<< channel.audioFile << " for channel: "
						<< channelName << std::endl;
				}else {
					std::cout << "[AudioSystem] Preloaded: "
						<< channel.audioFile << " for channel: "
						<< channelName
						<< (channel.isSpatial ? " (3D)" : " (2D)") << std::endl;
				}

			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Audio Scene Init", ms });
}

void AudioSystem::updateSpatialAudio(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		// Skip objects without audio or transform cus they are not spatial
		if (!obj->hasComponent<AudioComponent>() || !obj->hasComponent<Transform>()) continue;

		Transform* transform = obj->getComponent<Transform>();
		AudioComponent* audioC = obj->getComponent<AudioComponent>();

		// Update position for each 3D audio channel
		for (auto& [channelName, audio] : audioC->audioChannels) {
			// Only update 3D sounds that are currently playing
			if (!audio.isSpatial || audio.state != AudioState::Playing || !audio.channel) continue;

			// Set the 3D position for this sound
			FMOD_VECTOR soundPos = {
				transform->x,
				transform->y,
				transform->z
			};
			FMOD_VECTOR soundVel = { 0.0f, 0.0f, 0.0f };  // No doppler effect
			
			FMOD_RESULT result = audio.channel->set3DAttributes(&soundPos, &soundVel);

			if (result != FMOD_OK) {
				std::cerr << "[AudioSystem] Failed to set 3D attributes for: "
					<< obj->getObjectName() << " channel: " << channelName << std::endl;
			}
		}
	}
}

void TransformSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	if (EditorManager::isEditingMode()) return;

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	// This is a simple sort to ensure parents are processed before children in one pass
	// Roots first, then children. should handle single-level correctly
	// For multi-level, a full topological sort would be needed
	std::stable_partition(gameObjects.begin(), gameObjects.end(), [](GameObject* obj) {
		Transform* t = obj->getComponent<Transform>();
		return !t || t->parentName.empty(); // roots first
		});

	for (GameObject* object : gameObjects) {
		if (!object->hasComponent<Transform>()) continue;
		Transform* t = object->getComponent<Transform>();
		if (t->parentName.empty()) continue;

		GameObject* parent = manager.getGameObject(t->parentName);
		if (!parent || !parent->hasComponent<Transform>()) continue;

		Transform* pt = parent->getComponent<Transform>();
		float parentRad = glm::radians(pt->rotation);
		float cosA = cosf(parentRad);
		float sinA = sinf(parentRad);
		float offsetX = t->localOffset.x * (t->flipX ? -1.0f : 1.0f); // account for flipX
		float offsetY = t->localOffset.y;

		t->x = pt->x + offsetX * cosA - offsetY * sinA;
		t->y = pt->y + offsetX * sinA + offsetY * cosA;
		t->rotation = pt->rotation + t->localRotation;

		// Keep physics position in sync if child has physics
		if (object->hasComponent<Physics>()) {
			Physics* p = object->getComponent<Physics>();
			p->dynamics.position.x = t->x;
			p->dynamics.position.y = t->y;
		}
	}
}