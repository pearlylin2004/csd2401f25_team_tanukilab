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

/*!***********************************************************************
\brief
	enum class for shapes

*************************************************************************/
enum class shape
{
	square = 0,
	circle,
	triangle
};

/*!***********************************************************************
\brief
	struct for everything to do with rendering

*************************************************************************/
struct renderer
{
	/*!***********************************************************************
	\brief
		initializes the setups for the meshes and shader programs

	\param[in] w
		width of screen

	\param[in] h
		height of screen

	*************************************************************************/
	static void init(int w, int h);

	/*!***********************************************************************
	\brief
		struct for everything to do with meshes

	*************************************************************************/
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
	/*!***********************************************************************
	\brief
		storing of shader program
	*************************************************************************/
	static GLuint shdr_pgm;
	/*!***********************************************************************
	\brief
		setting up square mesh

	\param[in] clr
		taking in RGB colors

	\return model
		returns a mesh

	*************************************************************************/
	static model setup_square(glm::vec3 clr);
	/*!***********************************************************************
	\brief
		setting up circle mesh

	\param[in] clr
		taking in RGB colors

	\param[in] slices
		the amount of slices/amount of triangles in the circle

	*************************************************************************/
	static model setup_circle(glm::vec3 clr, int slices);
	/*!***********************************************************************
	\brief
		loading of shader programs into the system

	*************************************************************************/
	static void setup_shdrpgm();//const glm::mat4& p_modelMat);
	/*!***********************************************************************
	\brief
		array of all the available meshes

	*************************************************************************/
	static std::vector<model>models;

	/*!***********************************************************************
	\brief
		struct that contains all the data for a camera

	*************************************************************************/
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
		/*!***********************************************************************
		\brief
			initializes all the data for the camera

		\param[in] w
			width of the screen

		\param[in] h
			height of the screen

		*************************************************************************/
		void init(int w, int h);
		/*!***********************************************************************
		\brief
			updating view and orthographic projection matrix

		*************************************************************************/
		void update();
	};

	/*!***********************************************************************
	\brief
		global camera

	*************************************************************************/
	static camera cam;
};