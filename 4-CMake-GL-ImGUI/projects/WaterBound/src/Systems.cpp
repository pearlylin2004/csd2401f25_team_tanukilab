/* Start Header ************************************************************************/
/*!
\file        Systems.cpp
\author      Seow Sin Le, s.sinle, 2401084, 45%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 15%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
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
void InputSystem::update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus) {
	(void)deltaTime;
	(void)messageBus;
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

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
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "W", true }));
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_S)) {
					velocity->velY = -input->moveSpeed;
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "S", true }));
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
					velocity->velX = -input->moveSpeed;
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "A", true }));
				}
				if (InputHandler::isKeyHeld(GLFW_KEY_D)) {
					velocity->velX = input->moveSpeed;
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "D", true }));
				}

				if (InputHandler::isComboKeyTriggered(GLFW_KEY_0)) {
					UISystem::toggleUI();
					messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "0", true }));
				}

				// Press F9 to simulate a failure (for crash logger)
				if (InputHandler::isKeyTriggered(GLFW_KEY_F9)) {
					throw std::runtime_error("F9 test: simulated failure in update()");
				}
				
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Input", ms }); //saving timing for UI output
}


// Render system - draws all game objects with a Render component
void RenderSystem::update(GameObjectManager& manager, float const& deltaTime) 
{
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	glm::mat4 camView, camProj;

	// Attach texture to FBO and update editor camera if editor is on
	if (UISystem::isShowUI()) {
		createFBO();
		renderer::editorCam.update();

		camView = renderer::editorCam.view;
		camProj = renderer::editorCam.proj;
	}
	// else update game camera
	else {
		renderer::cam.update();

		camView = renderer::cam.view;
		camProj = renderer::cam.proj;
	}

	// Update textures if they have changed (in level editor)
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Render>())
		{
			Render* render = object->getComponent<Render>();

			if (render->texChanged) {
				if (render->hasTex) {
					glDeleteTextures(1, &render->texHDL); // to avoid leak
					render->texHDL = 0;
				}
				render->texHDL = uploadtex(render->texFile, render->isTransparent);
				render->texChanged = false;
			}
		}
	}


	std::vector<GameObject*> objectWithTex;
	std::vector<GameObject*> objectWithoutTex;

	// Update textures if they have changed (in level editor)
	for (GameObject* object : gameObjects)
	{
		if (object->hasComponent<Render>())
		{
			Render* render = object->getComponent<Render>();

			if (render->texChanged) {
				if (render->hasTex) {

					//glDeleteTextures(1, &render->texHDL); // to avoid leak
					//render->texHDL = 0;
					//no more deleting textures here, handled in ResourceManager cus we have a cache now

					//the below code is for future use
					TextureData texData = ResourceManager::getInstance().getTexture(render->texFile);
					render->texHDL = texData.id;
					render->isTransparent = texData.isTransparent;
					render->texChanged = false;
				}
				//render->texHDL = uploadtex(render->texFile, render->isTransparent);
				TextureData texData = ResourceManager::getInstance().getTexture(render->texFile);
				render->texHDL = texData.id;
				render->isTransparent = texData.isTransparent;

				render->texChanged = false;
			}

			if (object->getComponent<Render>()->hasTex)
				objectWithTex.push_back(object);
			else
				objectWithoutTex.push_back(object);
		}
	}

	//rendering objects without texture, just color
	glUseProgram(renderer::shdr_pgm[1]);
	GLint vTransformView = glGetUniformLocation(renderer::shdr_pgm[1], "V");
	glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(camView));
	GLint vTransformProj = glGetUniformLocation(renderer::shdr_pgm[1], "P");
	glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(camProj));
	for (GameObject* object : objectWithoutTex)
		renderNoTex(object);

	//rendering objects with textures
	glUseProgram(renderer::shdr_pgm[0]);
	vTransformView = glGetUniformLocation(renderer::shdr_pgm[0], "V");
	glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(camView));
	vTransformProj = glGetUniformLocation(renderer::shdr_pgm[0], "P");
	glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(camProj));
	for (GameObject* object : objectWithTex)
		renderTex(object, deltaTime);

	/*if (UISystem::isShowUI()) glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	//record end time for performance tracking
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Render", ms }); //saving timing for UI output

}

void RenderSystem::createFBO() {
	if (texture) glDeleteTextures(1, &texture);
	if (fbo) glDeleteFramebuffers(1, &fbo);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}

RenderSystem::~RenderSystem() {
	if (texture) glDeleteTextures(1, &texture);
	if (fbo) glDeleteFramebuffers(1, &fbo);
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
	}

	fboWidth = fboW;
	fboHeight = fboH;
	Font::init();
}

//this moved to ResourceManager.cpp
// Upload a texture from file and return its OpenGL handle
//GLuint RenderSystem::uploadtex(std::string const& filename, bool& isTransparent)
//{
//	int width, height, channels;
//	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
//	if (!data) {
//		std::cerr << "Failed to load image: " << filename << std::endl;
//		return 0;
//	}
//
//	std::cout << "Loaded " << filename
//		<< " (" << width << "x" << height << "), "
//		<< channels << " channels" << std::endl;
//
//	isTransparent = (channels == 4);
//
//	GLuint texobj_hdl;
//	glCreateTextures(GL_TEXTURE_2D, 1, &texobj_hdl);
//	glTextureStorage2D(texobj_hdl, 1, GL_RGBA8, static_cast<GLuint>(width), static_cast<GLuint>(height));
//	glTextureSubImage2D(texobj_hdl, 0, 0, 0, static_cast<GLuint>(width), static_cast<GLuint>(height), GL_RGBA, GL_UNSIGNED_BYTE, data);
//
//	glTextureParameteri(texobj_hdl, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTextureParameteri(texobj_hdl, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	stbi_image_free(data);
//	return texobj_hdl;
//}

void RenderSystem::renderTex(GameObject* object, float const& deltaTime)
{
	if (object->hasComponent<Render>() && object->hasComponent<Transform>())
	{
		Transform* transform = object->getComponent<Transform>();
		Render* render = object->getComponent<Render>();

		//position and orientation of mesh
		float angle = glm::radians(transform->rotation);
		transform->mdlWorld = glm::mat4{ 1 };
		transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
		transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
		transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));
		glBindTextureUnit(0, render->texHDL);
		//std::cerr << "Binding Texture ID: " << render->texHDL << " for object: " << object->getObjectName() << std::endl;

		glm::mat4 texRot{ 1 };
		texRot = glm::rotate(texRot, angle, glm::vec3(0, 0, 1.f));
		glm::vec2 rotationCenter{ 0,0 }; //if needed in the future it can be a member variable, now just for sanity

		//binding model to world matrix
		glBindVertexArray(render->modelRef.vaoid);
		GLint vTransformLoc = glGetUniformLocation(renderer::shdr_pgm[0], "M");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(transform->mdlWorld));

		glm::vec2 texpos, texscale;
		if (render->hasAnimation)
		{
			Animation* animation = object->getComponent<Animation>();
			texpos = glm::vec2{ animation->currentFrameColumn / static_cast<float>(animation->totalColumn), animation->currentFrameRow / static_cast<float>(animation->totalRow) };
			texscale = glm::vec2{ 1.f / animation->totalColumn, 1.f / animation->totalRow };
			if (animation->loop)
			{
				animation->frameTimer += deltaTime;
				//std::cout << "\n-------Frame Timer---------\n" << animation->frameTimer << "\n\n";
				if (animation->frameTimer >= animation->frameTime)
				{
					animation->frameTimer -= animation->frameTime;
					/*if (animation->runItBack && animation->currentFrameColumn == 0)
					{
						animation->runItBack = false;
					}
					else
					{
						animation->currentFrameColumn = (animation->currentFrameColumn + 1) % animation->totalColumn;
						animation->currentFrameRow = (animation->currentFrameRow + 1) % animation->totalRow;
						animation->runItBack = true;
					}*/
					animation->currentFrameColumn = (animation->currentFrameColumn + 1) % animation->totalColumn;
					animation->currentFrameRow = (animation->currentFrameRow + 1) % animation->totalRow;
					animation->runItBack = true;
					
				}
			}
		}
		else
		{
			texpos = glm::vec2{ 0,0 };
			texscale = glm::vec2{ 1,1 };
		}
		//binding texture rotation and id
		GLint texid = glGetUniformLocation(renderer::shdr_pgm[0], "uTex2d");
		GLint rotmat = glGetUniformLocation(renderer::shdr_pgm[0], "uRotMtx");
		GLint rotcenter = glGetUniformLocation(renderer::shdr_pgm[0], "uMcn");
		GLint offset = glGetUniformLocation(renderer::shdr_pgm[0], "uTexOffSet");
		GLint tscale = glGetUniformLocation(renderer::shdr_pgm[0], "uTexScale");
		glUniform1i(texid, 0);
		glUniformMatrix2fv(rotmat, 1, GL_FALSE, glm::value_ptr(texRot));
		glUniform2fv(rotcenter, 1, glm::value_ptr(rotationCenter));
		glUniform2fv(offset, 1, glm::value_ptr(texpos));
		glUniform2fv(tscale, 1, glm::value_ptr(texscale));

		if (render->modelRef.shape == shape::square)
		{
			// --- Pass 1: textured fill ---
			glBindVertexArray(render->modelRef.vaoid);
			glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
		}
		//glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
		else if (render->modelRef.shape == shape::circle)
			glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
		glBindVertexArray(0);
	}
}

void RenderSystem::renderNoTex(GameObject* object)
{
	if (object->hasComponent<Render>() && object->hasComponent<Transform>())
	{
		Transform* transform = object->getComponent<Transform>();
		Render* render = object->getComponent<Render>();

		//position and orientation of mesh
		float angle = glm::radians(transform->rotation);
		transform->mdlWorld = glm::mat4{ 1 };
		transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
		transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
		transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));
		glBindTextureUnit(0, 0);

		glm::mat4 texRot{ 1 };
		texRot = glm::rotate(texRot, angle, glm::vec3(0, 0, 1.f));
		glm::vec2 rotationCenter{ 0,0 }; //if needed in the future it can be a member variable, now just for sanity

		//binding model to world matrix
		glBindVertexArray(render->modelRef.vaoid);
		GLint vTransformLoc = glGetUniformLocation(renderer::shdr_pgm[1], "M");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(transform->mdlWorld));

		if (render->modelRef.shape == shape::square)
			glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
		else if (render->modelRef.shape == shape::circle)
			glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
		glBindVertexArray(0);
	}
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

	ResourceManager::getInstance().getFont("shaders/Orange Knight.ttf");
	ResourceManager::getInstance().getFont("shaders/ARIAL.TTF");
	ResourceManager::getInstance().getFont("shaders/times.ttf");

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

void FontSystem::update(GameObjectManager& manager)
{
	//for debugging only

	//std::cout << "=== FontSystem::update() called ===" << std::endl;

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
	case 0: fontPath = "shaders/Orange Knight.ttf"; break;
	case 1: fontPath = "shaders/ARIAL.TTF"; break;
	case 2: fontPath = "shaders/times.ttf"; break;
	default: fontPath = "shaders/ARIAL.TTF"; break;
	}

	const FontData& fontData = ResourceManager::getInstance().getFont(fc->fontPath);

	/*std::cout << "FontData has " << fontData.characters.size() << " characters loaded" << std::endl;
	if (fontData.characters.empty()) {
		std::cerr << "ERROR: Font data has NO characters!" << std::endl;
		return;
	}*/

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
}

// Physics system - updates position based on velocity and applies gravity
void PhysicsSystem::update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus) {
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

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
				messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "SPACE", true }));
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
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Physics", ms }); //saving timing for UI output
}

// Collision system - detects and resolves collisions between game objects
void CollisionSystem::update(GameObjectManager& manager, const float& deltaTime) {
	(void)deltaTime;
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

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
				//for output into console for debugging and prove
				UI::addDebugMessage("Object " + obj->getObjectName()
					+ " inserted into cell (" + std::to_string(x) + "," + std::to_string(y) + ")\n");
			}
		}
	}
	//temporary container to track checked objects for this frame
	std::unordered_map<GameObject*, std::vector<GameObject*>> checked;

	//checking collision 
	for (int i = 0; i < GRID_WIDTH; i++) {
		for (int j = 0; j < GRID_HEIGHT; j++) {
			auto& cellObject = grid[i][j].objects; //get objects in the cell
			//loop through objects in the cell to check for collision
			for (size_t n = 0; n < cellObject.size(); n++) {
				//object 1
				GameObject* obj1 = cellObject[n];
				if (!obj1->hasComponent<Physics>()) continue;
				Transform* t1 = obj1->getComponent<Transform>();
				Collision::AABB aabb1 = Collision::getObjectAABB(t1);
				Physics* p1 = obj1->getComponent<Physics>();
				Vector2D vel1{ p1->velX, p1->velY };
				//object 2 looping
				for (size_t m = n + 1; m < cellObject.size(); m++) {
					//object 2
					GameObject* obj2 = cellObject[m];
					//message to check for collision between objects
					UI::addDebugMessage("Checking collision: " + obj1->getObjectName()
						+ " vs " + obj2->getObjectName() + "\n");
					if (!obj2->hasComponent<Physics>()) continue;

					//skip if already checked
					auto it = std::find(checked[obj1].begin(), checked[obj1].end(), obj2);
					if (it != checked[obj1].end()) continue;
					//mark as checked both ways
					checked[obj1].push_back(obj2);
					checked[obj2].push_back(obj1);

					Transform* t2 = obj2->getComponent<Transform>();
					Collision::AABB aabb2 = Collision::getObjectAABB(t2);
					Physics* p2 = obj2->getComponent<Physics>();
					Vector2D vel2{ p2->velX, p2->velY };

					CollisionInfo info = Collision::CollisionIntersection_RectRect_Dynamic_Info(aabb1, vel1, aabb2, vel2); //check collision
					
					//checking if collided
					if (info.collided) {
						//update the velocity and penetrations
						if (info.normal.x != 0) {
							t1->x += info.normal.x * info.penetration;
							p1->velX = 0; //stop horizontal movement
						}

						// resolve Y
						if (info.normal.y != 0) {
							t1->y += info.normal.y * info.penetration;
							p1->velY = 0; //stop vertical movement
						}				
						UI::addDebugMessage("Collision detected between Object.\n"); //message for debugger
					}
				}
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Collisions", ms }); //saving timing for UI output
}

//UI system - handles ImGui rendering and interaction
bool UISystem::showUI = false;

void UISystem::init(GLFWwindow* window, RenderSystem* renderer) {
	// --- ImGui Setup ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.FontGlobalScale = 2.0f;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_DockingEnable
			| ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
	// io.IniFilename = "imgui_layout.ini";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	// no show task bar icon for additional viewports
	io.ConfigViewportsNoTaskBarIcon = true;

	m_UI.loadAssetsFromDirectory();

	m_renderer = renderer;
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

	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	dockingSetUp();

	// get the width and height of the SCENE view
	int fboWidth, fboHeight;
	m_renderer->fboAspectRatio(fboWidth, fboHeight);

	// all ImGui windows
	m_UI.update(manager);
	m_UI.sceneWindow(m_renderer->getTexture(), static_cast<float>(fboWidth) / static_cast<float>(fboHeight));

	m_UI.loadAssetsFromDirectory(); // only reload if assets flag is set (e.g. new asset added)

	// this will CRASH if change scene in editor
	//Transform* leavesT = gameObjects[0]->getComponent<Transform>();
	//Transform* bulletT = gameObjects[2]->getComponent<Transform>();
	//m_UI.shapeControl(*leavesT, *bulletT); 

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	renderMultipleViewPorts();

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "IMGUI", ms }); //saving timing for UI output
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
		ImGui::DockBuilderRemoveNode(dockspaceID); // Clear any existing layout
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

		// Split the dockspace into major areas
		ImGuiID dockParent = dockspaceID;
		ImGuiID dockDown, dockRight, dockLeft;
		ImGuiID dockRightSecond;

		ImGui::DockBuilderSplitNode(dockParent, ImGuiDir_Left, 0.3f, &dockLeft, &dockParent);
		ImGui::DockBuilderSplitNode(dockParent, ImGuiDir_Right, 0.4f, &dockRight, &dockParent);
		ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.4f, &dockRightSecond, &dockRight);
		ImGui::DockBuilderSplitNode(dockParent, ImGuiDir_Down, 0.4f, &dockDown, &dockParent);

		// Dock windows

		// Left
		ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
		ImGui::DockBuilderDockWindow("Inspector", dockRight);
		ImGui::DockBuilderDockWindow("Editor Camera", dockRight);

		// Right
		ImGui::DockBuilderDockWindow("Create Object", dockRightSecond);

		// Middle
		ImGui::DockBuilderDockWindow("Scene", dockParent);
		
		// Bottom
		ImGui::DockBuilderDockWindow("Performance", dockDown);
		ImGui::DockBuilderDockWindow("Debug Info", dockDown);
		ImGui::DockBuilderDockWindow("Assets", dockDown);

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

void LogicSystem::update(GameObjectManager& manager, float const& dt) {

	std::vector<GameObject*> objs;
	manager.getAllGameObjects(objs);

	static LogicContainer container; // stateless helper

	for (auto* go : objs) {
		if (!go) continue;
		if (!go->hasComponent<StateMachine>()) continue;
		if (!go->hasComponent<Transform>() || !go->hasComponent<Physics>()) continue;

		container.update(*go, dt, manager); 
	}
}
