/* Start Header ************************************************************************/
/*!
\file        Systems.cpp
\author      Seow Sin Le, s.sinle, 2401084, 20%
\par         s.sinle@digipen.edu
\date        October, 1st, 2025
\brief       System functions to run the logic for the components

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
void InputSystem::update(GameObjectManager& manager, float deltaTime) {
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
				//so far so good i think?
			}
		}
	}
}

void RenderSystem::update(GameObjectManager& manager, float deltaTime) {
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


void PhysicsSystem::update(GameObjectManager& manager, float deltaTime) {
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
				std::cout << "bullet is spawned\n";
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
				p->velX = 2.0f;
				p->velY = 0.0f;
				p->onGround = false;
			}

			// Jump
			if (InputHandler::isKeyTriggered(GLFW_KEY_B)){// && physics->onGround) {
				std::cout << "player is jumping\n";
				physics->velY += physics->jumpForce;
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


//collision system
void CollisionSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* iterator : gameObjects) {
		if (iterator->hasComponent<CollisionInfo>() && iterator->hasComponent<Physics>()) {

			for (GameObject* obj : gameObjects) {
				if (obj != iterator) {
					if (obj != iterator && obj->getComponent<CollisionInfo>()) {
						/*if (!obj->hasComponent<Transform>() || !iterator->hasComponent<Transform>()) {

						}*/
						Transform* obj1T = obj->getComponent<Transform>();
						Transform* obj2T = iterator->getComponent<Transform>();
						Collision::AABB obj1 = Collision::getObjectAABB(obj1T);
						Collision::AABB obj2 = Collision::getObjectAABB(obj2T);
						Physics* obj1V = obj->getComponent<Physics>();
						Physics* obj2V = iterator->getComponent<Physics>();
						Vector2D obj1Vel{ obj1V->velX, obj1V->velY };
						Vector2D obj2Vel{ obj2V->velX, obj2V->velY };
						CollisionInfo info = Collision::CollisionIntersection_RectRect_Dynamic_Info(obj1, obj1Vel, obj2, obj2Vel);

						if (info.collided) {
							Collision::Resolve_AABB_AABB(
								obj1, obj1Vel, 0.0f,
								obj2, obj2Vel, 1.0f,
								info, CollisionResponseMode::Stop
							);

							//change velocity
							obj1V->velX = obj1Vel.x;
							obj1V->velY = obj1Vel.y;
							obj2V->velX = obj2Vel.x;
							obj2V->velY = obj2Vel.y;

							//show message in imgui
							UI::debugMessages.push_back("Collision detected between Object " +
								std::to_string(reinterpret_cast<uintptr_t>(obj)) +
								" and Object " +
								std::to_string(reinterpret_cast<uintptr_t>(iterator)));
						}
					}
				}
			}
		}
	}
}

//for imgu(testing)
void UISystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	static float rotationSpeed = 0.01f;
	static float angleOfRotation = 0.0f;
	//demo
	UI::drawDemo(rotationSpeed, angleOfRotation);

	//input circles
	UI::drawControls(circles);

	//debug window
	UI::drawDebug(UI::debugMessages);

	//shape control
	/*for (GameObject* obj : gameObjects) {
		Transform* obj1T = obj->getComponent<Transform>();
		UI::shapeControl(*obj1T);
	}*/

	Transform* obj1T = gameObjects[0]->getComponent<Transform>();
	UI::shapeControl(*obj1T);

	//ImGui::ShowMetricsWindow();
}

