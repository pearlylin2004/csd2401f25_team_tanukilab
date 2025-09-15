#pragma once
#include <shader.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
	static void init(int, int);
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
		GLuint primitive_cnt;

	};

	static model setup_square(glm::vec3);
	static model setup_circle(glm::vec3, int);
	static void setup_shdrpgm();//const glm::mat4& p_modelMat);
	static void draw();
	static std::vector<model>models;

	struct object
	{
		glm::vec2 scale;
		glm::vec2 rot;
		glm::vec2 translate;
		model model_ref;
		std::string texture_ref;
		void init();
		void update();
		void draw();
	};

	struct camera
	{
		static GLfloat ar;
		static int width;
		static glm::vec2 campos;
		static int zoom;
		void init();
		void update(int w, int h);
	};

	static std::vector<object> objects;
	static void modelxform();
	
};