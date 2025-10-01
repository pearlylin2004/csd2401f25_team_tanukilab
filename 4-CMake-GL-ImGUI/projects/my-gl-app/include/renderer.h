/* Start Header ************************************************************************/
/*!
\file        renderer.h
\author      Seow Sin Le, s.sinle, 2401084
\par         s.sinle@digipen.edu
\date        October, 1st, 2025
\brief       initializes the meshes and camera

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
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
#include <map>
#include <input.h>

enum class shape
{
	square = 0,
	circle,
	triangle
};

struct renderer
{
	static void init(int, int);

	struct model
	{
		shape shape;
		GLenum primitive_type;
		GLuint vaoid;
		GLuint vbo;
		GLuint ebo;
		GLuint elem_cnt;
		GLuint draw_cnt;
		GLuint primitive_cnt;

	};

	static GLuint shdr_pgm;
	static model setup_square(glm::vec3);
	static model setup_circle(glm::vec3, int);
	static void setup_shdrpgm();//const glm::mat4& p_modelMat);
	//static void draw();
	static std::vector<model>models;

	struct camera
	{
		GLfloat ar;
		int width;
		glm::vec3 campos;
		float zoom;
		float rot;
		glm::vec3 up;
		glm::mat4 view;
		glm::mat4 proj;
		void init(int w, int h);
		void update();
	};

	//static std::map<std::string, object> objects;
	static camera cam;
};