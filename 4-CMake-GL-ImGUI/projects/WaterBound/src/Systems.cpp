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

	//for (GameObject* iterator : gameObjects) {
	//	//check for objects that have input 
	//	if (iterator->hasComponent<Input>()) {
	//		Input* input = iterator->getComponent<Input>();
	//		if (iterator->hasComponent<Physics>()) {//player got that velocity in him
	//			Physics* velocity = iterator->getComponent<Physics>();


	//			if (InputHandler::isKeyHeld(GLFW_KEY_W)) {
	//				velocity->velY = input->moveSpeed;
	//			}
	//			if (InputHandler::isKeyHeld(GLFW_KEY_S)) {
	//				velocity->velY = -input->moveSpeed;
	//			}
	//			if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
	//				velocity->velX = -input->moveSpeed;
	//			}
	//			if (InputHandler::isKeyHeld(GLFW_KEY_D)) {
	//				velocity->velX = input->moveSpeed;
	//			}
#ifdef _DEBUG
	if (InputHandler::isComboKeyTriggered(GLFW_KEY_0)) {
		UISystem::toggleUI();

		// refresh cached object vector
		CachedGameObjects::Instance().refresh();

		messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "0", true }));

		//AudioHandler::getInstance().toggleSoundPause(soundID::bg);

		if (EditorManager::isEditingMode()) EditorManager::toggleEditing(false);
		else EditorManager::toggleEditing(true);
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


	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Input", ms }); //saving timing for UI output
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
	//drawing models with texture
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
	instancePerBatch(objectWithTex2);
	for (std::pair<const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex2)
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
	glEnable(GL_BLEND);
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
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);
	for (std::pair < const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithTex) pair.second.clear();
	for (std::pair < const BatchKey, std::vector<renderer::InstanceData>>& pair : objectWithoutTex) pair.second.clear();

	for (GameObject* obj : gameObjects)
	{
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
		data.color = glm::vec4(render->clr, 1);
		data.texParams = glm::vec4(0, 0, 1, 1); // default, no texture frame

		// if obj has animation & state machine component
		if (obj->hasComponent<Animation>() && obj->hasComponent<StateMachine>())
		{
			Animation* animation = obj->getComponent<Animation>();
			StateMachine* sm = obj->getComponent<StateMachine>();

			AnimateState& as = animation->animState[static_cast<int>(sm->state)];
				
			if (render->hasAnimation && !as.texFile.empty()) {
				glm::vec2 texOffSet{ as.currentFrameColumn / static_cast<float>(as.totalColumn), 1.f - ((as.currentFrameRow + 1) / static_cast<float>(as.totalRow)) };
				glm::vec2 texScale{ 1.f / as.totalColumn, 1.f / as.totalRow };

				if (as.loop)
				{
					as.frameTimer += deltaTime;
					if (as.frameTimer >= as.frameTime && !EditorManager::isEditingMode() && !EditorManager::isPaused())
					{
						as.frameTimer -= as.frameTime;
						if (as.currentFrameColumn >= as.lastFrame.x && as.currentFrameRow >= as.lastFrame.y)
						{
							as.currentFrameColumn = (int)as.initialFrame.x;
							as.currentFrameRow = (int)as.initialFrame.y;
							as.frameTimer = 0;
						}
						else
						{
							++as.currentFrameColumn;

							if (as.currentFrameColumn >= as.totalColumn)
							{
								as.currentFrameColumn = 0;
								++as.currentFrameRow;

								if (as.currentFrameRow > as.lastFrame.y)
								{
									as.currentFrameColumn = (int)as.initialFrame.x;
									as.currentFrameRow = (int)as.initialFrame.y;
								}
							}
						}
						/*as.currentFrameColumn = (as.currentFrameColumn + 1) % as.totalColumn;
						as.currentFrameRow = (as.currentFrameRow + 1) % as.totalRow;*/
						animation->runItBack = true;
					}
				}
				data.texParams = { texOffSet,texScale };
				BatchKey key{ render->modelRef.shape, as.texHDL };
				objectWithTex[key].push_back(data);
			}
			// draw shape if obj texture file is empty
			else {
				BatchKey key{ render->modelRef.shape, 0 };
				objectWithoutTex[key].push_back(data);
			}
		}
		else if (render->hasTex)
		{
			BatchKey key{ render->modelRef.shape, render->texHDL };
			objectWithTex[key].push_back(data);
		}
		else
		{
			BatchKey key{ render->modelRef.shape, 0 };
			objectWithoutTex[key].push_back(data);
		}
	}

	
	//Font::init();
}

void RenderSystem::instancePerBatch(std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> &batch)
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
	}

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
		if (!physics->physicsFlag) continue;
		Transform* transform = object->getComponent<Transform>();

		// Handle player input
		if (object->hasComponent<Input>() && !EditorManager::isEditingMode() && !EditorManager::isPaused()) {
			if (!object->checkAutoMove()) {
				PhysicsForces::applyDamping(physics);
				PhysicsForces::updatePosition(transform, physics, deltaTime);
			}

			// Continuous shooting system
			if (physics->canShoot) {
				// Update shoot timer
				physics->shootTimer += deltaTime;

				// Check if mouse button is being held down
				if (InputHandler::isMouseLeftHeld()) {
					
					if (physics->shootTimer >= physics->fireRate) {
						GameObject* bullet = PhysicsForces::findAvailableBullet(manager);

						if (bullet != nullptr) {
							// Get mouse position in world coordinates
							Vector2D mouseWorldPos = InputHandler::getWorldMousePosition();

							// Shoot bullet toward mouse position
							PhysicsForces::shoot(bullet, transform, mouseWorldPos);
							messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "LEFT_CLICK", true }));

							if (bullet->hasComponent<AudioComponent>()) {
								AudioComponent* audio = bullet->getComponent<AudioComponent>();
								AudioChannel* ch = audio->getDefaultChannel();
								ch->isPendingPlay = true;
							}

							DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
							physics->shootTimer = 0.0f;
						}
						else {
							physics->shootTimer = physics->fireRate * 0.5f;
							DebugLog::addMessage("No bullets available!", DebugMode::PlaySimul);
						}
					}
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

			// Movement controls (A/D keys)
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
			physics->dynamics.velocity.x = targetVelX;

			// Jump (SPACE key)
			if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE)) {
				if (previousOnGround[object]) {
					PhysicsForces::jump(object);
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "SPACE", true }));
				}
			}

			// Ground collision
			if (transform->y < physics->floorY && !object->checkAutoMove()) {
				transform->y = physics->floorY;
				physics->dynamics.position.y = physics->floorY;
				physics->dynamics.velocity.y = 0.0f;
				physics->velY = 0.0f;
				physics->onGround = true;
			}
			transform->x += physics->velX * deltaTime;
		}

		// Auto-move objects
		if (object->checkAutoMove()) {
			transform->y += physics->dynamics.velocity.y * deltaTime;
			transform->x += physics->dynamics.velocity.x * deltaTime;
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
			// Update bullet physics
			Vector3D gravityForce = { 0.0f, p->gravity * p->dynamics.mass, 0.0f };

			DynamicsSystem::UpdateForces(p->dynamics, deltaTime);
			DynamicsSystem::Integrate(p->dynamics, deltaTime, p->gravity, false);

			t->x = p->dynamics.position.x;
			t->y = p->dynamics.position.y;
			// Update life timer
			p->lifeTimer += deltaTime;
			// Deactivate if lifetime exceeded
			if (p->lifeTimer >= p->maxLifetime || p->dynamics.position.y < -20.0f) {
				PhysicsForces::deactivateBullet(obj);
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Physics", ms });
}

// Collision system - detects and resolves collisions between game objects
void CollisionSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	double ms = 0.0;

	//get layer manager
	LayerManager& layerManager = manager.getLayerManager();
	std::vector<Layer*> layers = layerManager.getAllLayers();

	//initialize spatial grid
	const float CELL_SIZE = 4.0f; //changed grid size
	const int GRID_WIDTH = 20;
	const int GRID_HEIGHT = 20;

	//persistant grid for this layer
	static std::vector<std::vector<Collision::Cell>> grid(GRID_WIDTH, std::vector<Collision::Cell>(GRID_HEIGHT));

	//process each layer separately
	//right now only layer 1 should have any sort of collision
	for (Layer* layer : layers) {
		//skip missing layer
		if (!layer) {
			std::cerr << "CollisionSystem::update() - Layer " << layer->getLayerID() << " not found!\n";
			continue;
		}

		//clear grid instead of reallocating every frame
		for (auto& column : grid) {
			for (auto& cell : column) {
				cell.objects.clear();
			}
		}

		const std::vector<GameObject*>& layerObjects = layer->getObjects(); //get layer object

		//insert object into grid 
		for (GameObject* obj : layerObjects) {
			Transform* objT = obj->getComponent<Transform>();
			CollisionInfo* c = obj->getComponent<CollisionInfo>();
			if (!c || !objT || !c->collisionFlag) continue; //skip if object not suppose to collide, no transform and no collision

			int minCellX, maxCellX, minCellY, maxCellY;
			//square
			if (c->colliderType == shape::square){
				Collision::AABB box = Collision::getObjectAABBbyCollider(objT, c->colliderSize); // use custom collider instead of scale
				minCellX = static_cast<int>(box.getMin().x / CELL_SIZE);
				maxCellX = static_cast<int>(box.getMax().x / CELL_SIZE);
				minCellY = static_cast<int>(box.getMin().y / CELL_SIZE);
				maxCellY = static_cast<int>(box.getMax().y / CELL_SIZE);
			}
			//circle
			else if (c->colliderType == shape::circle) {
				//need one for circle
				Collision::Circle circle = Collision::getObjectCirclebyCollider(objT, c->colliderSize);

				float minX = circle.getCenter().x - circle.getRadius();
				float maxX = circle.getCenter().x + circle.getRadius();
				float minY = circle.getCenter().y - circle.getRadius();
				float maxY = circle.getCenter().y + circle.getRadius();

				minCellX = static_cast<int>(minX / CELL_SIZE);
				maxCellX = static_cast<int>(maxX / CELL_SIZE);
				minCellY = static_cast<int>(minY / CELL_SIZE);
				maxCellY = static_cast<int>(maxY / CELL_SIZE);
			}

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

		//temporary container to track checked objects for this frame
		//to advoid duplicate checks
		//std::unordered_map<GameObject*, std::vector<GameObject*>> checked;

		//checking collision 
		//record current time for performance tracking
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < GRID_WIDTH; i++) {
			for (int j = 0; j < GRID_HEIGHT; j++) {
				auto& cellObject = grid[i][j].objects; //get objects in the cell
				//loop through objects in the cell to check for collision
				for (size_t n = 0; n < cellObject.size(); n++) {
					//object 1
					GameObject* obj1 = cellObject[n];
					Transform* t1 = obj1->getComponent<Transform>();
					CollisionInfo* c1 = obj1->getComponent<CollisionInfo>();
					Physics* p1 = obj1->getComponent<Physics>();
					if (!p1 || !c1 || !t1) continue;
					
					Vector2D vel1{ p1->dynamics.velocity.x, p1->dynamics.velocity.y };
					//object 2 looping
					for (size_t m = n + 1; m < cellObject.size(); m++) {
						GameObject* obj2 = cellObject[m]; //get obj2

						//check if it have been check already
						if (obj1 >= obj2)
							continue;

						//object 2
						Transform* t2 = obj2->getComponent<Transform>();
						CollisionInfo* c2 = obj2->getComponent<CollisionInfo>();
						Physics* p2 = obj2->getComponent<Physics>();
						if (!p2 || !t2 || !c2) continue;

						Vector2D vel2{ p2->dynamics.velocity.x, p2->dynamics.velocity.y };

						// Skip if these collision tags shouldn't collide
						if (!c1->shouldCollideWith(c2->collisionTag) ||
							!c2->shouldCollideWith(c1->collisionTag)) {
							continue;
						}

						//check collision based on shape
						//check obj1 shape 
						if (!c1 || !t1) return;
						//check obj2 shape 
						if (!c2 || !t2) return;

						//collision info
						CollisionInfo info;

						//if both square
						if (c1->colliderType == shape::square && c2->colliderType == shape::square) {
							Collision::AABB aabb1 = Collision::getObjectAABBbyCollider(t1, c1->colliderSize);
							Collision::AABB aabb2 = Collision::getObjectAABBbyCollider(t2, c2->colliderSize);
							info = Collision::CollisionIntersection_RectRect_Dynamic_Info(aabb1, vel1, aabb2, vel2);
						}
						//if both circle
						else if (c1->colliderType == shape::circle && c2->colliderType == shape::circle) {
							Collision::Circle circle1 = Collision::getObjectCirclebyCollider(t1, c1->colliderSize);
							Collision::Circle circle2 = Collision::getObjectCirclebyCollider(t2, c2->colliderSize);
							info = Collision::CollisionIntersection_CircleCircle_Dynamic_Info(circle1, vel1, circle2, vel2);
						}
						// if one is square one is circle
						else if (c1->colliderType == shape::square && c2->colliderType == shape::circle) {
							Collision::AABB aabb1 = Collision::getObjectAABBbyCollider(t1, c1->colliderSize);
							Collision::Circle circle2 = Collision::getObjectCirclebyCollider(t2, c2->colliderSize);
							info = Collision::CollisionIntersection_CircleAABB_Dynamic_Info(circle2, vel2, aabb1, vel1);
						}		
						else {
							continue; //current dont have other shapes of collision 
						}

						//checking if collided
						if (info.collided) {
							// c1 is the moving obj (player), c2 is the obj it collide with
							// CASE 1: collided obj is pushable
							if (c2->collisionRes == CollisionResponseMode::MoveWhenCollide) {
								if (info.normal.x != 0) {
									// move both obj away from each other (to simulate push)
									t1->x += info.normal.x * info.penetration * 0.5f;
									t2->x -= info.normal.x * info.penetration * 0.5f;

									p1->dynamics.position.x = t1->x;
									p2->dynamics.position.x = t2->x;
								}

								if (info.normal.y != 0) {
									t1->y += info.normal.y * info.penetration * 0.5f;
									t2->y -= info.normal.y * info.penetration * 0.5f;

									// Update physics positions to match transforms
									p1->dynamics.position.y = t1->y;
									p2->dynamics.position.y = t2->y;

									p1->dynamics.velocity.y = 0.f;
									p2->dynamics.velocity.y = 0.f;

									if (info.normal.y > 0) p1->onGround = true;
									else p2->onGround = true;
								}
							}
							// CASE 2: collided obj is static
							else if (c2->collisionRes == CollisionResponseMode::StopWhenCollide) {
								// Resolve penetration
								if (info.normal.x != 0) {
									// push moving obj out of collided obj
									t1->x += info.normal.x * info.penetration;
									p1->dynamics.position.x = t1->x;

									p1->dynamics.velocity.x = 0.f;
									p1->velX = 0.f;

								}

								if (info.normal.y != 0) {
									t1->y += info.normal.y * info.penetration;
									p1->dynamics.position.y = t1->y;

									p1->dynamics.velocity.y = 0.0f;

									if (info.normal.y > 0) p1->onGround = true;
								}

								DebugLog::addMessage("Collision detected between Object.\n", DebugMode::PlaySimul);
							}
							// CASE 3: anything else
							else {
								if (info.normal.y != 0) {
									p1->dynamics.velocity.y = 0.0f;
									p2->dynamics.velocity.y = 0.0f;

									if (info.normal.y > 0) p1->onGround = true;
									else p2->onGround = true;
								}
							}
						}
					}
				}
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		ms += std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	}
	g_SystemTimers.push_back({ "Collisions", ms }); //saving timing for UI output
}

#ifdef _DEBUG
//UI system - handles ImGui rendering and interaction
bool UISystem::showUI = false;

void UISystem::init(GLFWwindow* window, RenderSystem* renderer, GameObjectManager& manager) {
	// --- ImGui Setup ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.FontGlobalScale = 2.0f;
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

	static LogicContainer container; // Single container instance

	for (auto* go : objs) {
		if (!go) continue;
		if (!go->hasComponent<StateMachine>()) continue;
		if (!go->hasComponent<Transform>() || !go->hasComponent<Physics>()) continue;

		// Update class-based FSM + trigger-action rules
		container.update(*go, dt);
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Logic", ms });
}
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
				if (ch.playOnStart) {
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
				// Pre-load the sound via ResourceManager
				FMOD::Sound* sound = ResourceManager::getInstance().getSound(channel.audioFile);

				if (!sound) {
					std::cerr << "[AudioSystem] Failed to preload: "
						<< channel.audioFile << " for channel: "
						<< channelName << std::endl;
				}
				else {
					std::cout << "[AudioSystem] Preloaded: "
						<< channel.audioFile << " for channel: "
						<< channelName << std::endl;
				}
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Audio Scene Init", ms });
}
