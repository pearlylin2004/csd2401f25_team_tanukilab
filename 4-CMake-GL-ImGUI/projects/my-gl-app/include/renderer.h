#pragma once
#include <shader.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include "openGLwindow.h"

enum class shape
{
	square = 0,
	circle,
	triangle
};

struct renderer
{
	static void init();
	static void update(GLFWwindow*);
	static void cleanup();

	

	struct model
	{
		shape shape;
		GLuint shdr_pgm;
		GLenum primitive_type;
		GLuint vaoid;
		GLuint vbo;
		GLuint ebo;
		GLuint elem_cnt;
		GLuint draw_cnt;

	};

	static model setup_square(glm::vec3);
	static void setup_shdrpgm();//const glm::mat4& p_modelMat);
	static void draw();
	static std::vector<model>models;
	
};