#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

struct renderer
{
	static void init();
	static void update();
	static void draw();
	static void cleanup();

	void setup_vao();
	void setup_shdrpgm();
	void draw();

	struct model
	{
		GLuint vaoid;
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
	};
	
};