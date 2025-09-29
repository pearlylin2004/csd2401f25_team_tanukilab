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

				//reset to 0 every frame
				velocity->velX = 0.0f;
				velocity->velY = 0.0f;

				if (InputHandler::isKeyDown(GLFW_KEY_W)) {
					velocity->velY = input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_S)) {
					velocity->velY = -input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_A)) {
					velocity->velX = -input->moveSpeed;
				}
				if (InputHandler::isKeyDown(GLFW_KEY_D)) {
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

			//position and orientation of mesh
			float angle = glm::radians(transform->rotation);
			transform->mdlWorld = glm::mat4{ 1 };
			transform->mdlWorld = glm::translate(transform->mdlWorld, glm::vec3(transform->x, transform->y, transform->z));
			transform->mdlWorld = glm::rotate(transform->mdlWorld, angle, glm::vec3(0, 0, 1.f));
			transform->mdlWorld = glm::scale(transform->mdlWorld, glm::vec3(transform->scaleX, transform->scaleY, transform->scaleZ));

			//texture bs
			render->texHDL = uploadtex(render->texFile, render->resolution);
			glBindTextureUnit(0, render->texHDL);
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
			glUniform1i(texid, 0);
			glUniformMatrix2fv(rotmat, 1, GL_FALSE, glm::value_ptr(texRot));
			glUniform2fv(rotcenter, 1, glm::value_ptr(rotationCenter));
			glUniform1i(hastex, static_cast<int>(render->hasTex));

			if (render->modelRef.shape == shape::square)
				glDrawElements(GL_TRIANGLES, render->modelRef.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
			else if (render->modelRef.shape == shape::circle)
				glDrawArrays(render->modelRef.primitive_type, 0, render->modelRef.draw_cnt);
			glBindVertexArray(0);

		}
	}
}

GLuint RenderSystem::uploadtex(std::string const& filename, std::pair<int const, int const> resolution)
{
	GLuint width{ static_cast<GLuint>(resolution.first) }, height{ static_cast<GLuint>(resolution.second) };
	std::ifstream ifs{ filename, std::ios::binary | std::ios::ate };
	if (!ifs)
	{
		std::cerr << "Failed to open texture file: " << filename << std::endl;
		return 0;
	}
	size_t texel_file_size = static_cast<size_t>(ifs.tellg()); //get file size
	ifs.seekg(0, std::ios::beg); //back to start
	unsigned char* ptr_texels = new unsigned char[texel_file_size];
	if (ifs.read(reinterpret_cast<char*>(ptr_texels), texel_file_size)) {
		std::cout << "Loaded " << texel_file_size << " bytes from " << filename << std::endl;
	}
	else {
		std::cerr << "Error reading file contents." << std::endl;
	}

	GLuint texobj_hdl;
	glCreateTextures(GL_TEXTURE_2D, 1, &texobj_hdl);
	glTextureStorage2D(texobj_hdl, 1, GL_RGBA8, width, height);
	glTextureSubImage2D(texobj_hdl, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ptr_texels);
	delete[] ptr_texels;
	ifs.close();
	return texobj_hdl;
}

void PhysicsSystem::update(GameObjectManager& manager, float deltaTime) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for(auto object : gameObjects) {
		if (object->hasComponent<Transform>() && object->hasComponent<Physics>()) {
			Transform* transform = object->getComponent<Transform>();
			Physics* physics = object->getComponent<Physics>();
			transform->x += physics->velX * deltaTime;
			transform->y += physics->velY * deltaTime;
		

		/*if (object->hasComponent<Physics>() && !object->hasComponent<Input>()) {
			*/
			/*Physics* physics = object->getComponent<Physics>();*/

		if (InputHandler::isKeyDown(GLFW_KEY_B)) { ///Needa update
			if (!physics->inWaterTriggered) {
				physics->velY = physics->waterBounce;     // instant upward push
				physics->inWaterTriggered = true;
			}
		}

		if (physics->inWaterTriggered) {
			// Apply gravity and damping only after entering water
			physics->velY += (physics->gravity * 0.3f) * deltaTime;
			physics->velY *= physics->damping;




			transform->y += physics->velY * deltaTime;

			// Floor collision
			if (transform->y< physics->floorY) {
				transform->y = physics->floorY;
				physics->velY = 0.0f;

				physics->inWaterTriggered = false;
			}
		}
		}
	}
}