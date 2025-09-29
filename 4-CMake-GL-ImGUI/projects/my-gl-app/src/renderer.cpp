#include "renderer.h"

std::vector<renderer::model> renderer::models;
//std::map<std::string, renderer::object> renderer::objects;
GLuint renderer::shdr_pgm;
renderer::camera renderer::cam;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void renderer::init(int w, int h)
{
	cam.init(w, h);
	models.push_back(setup_square(glm::vec3(1.f, 1.f, 1.f)));
	models.push_back(setup_circle(glm::vec3(0.5f, 0.5f, 0.5f), 20));
	/*object obj1;
	objects.emplace("player", obj1);
	objects.emplace("platform",obj1);
	objects["player"].model_ref = models[0];
	objects["platform"].model_ref = models[1];
	for (std::map<std::string, object>::iterator i = objects.begin(); i != objects.end(); ++i)
	{
		std::cout << (*i).first << "\n";
		(*i).second.init();
		if ((*i).first == "player")
			i->second.translate.z = 0.01;
		(*i).second.update();
	}*/
	setup_shdrpgm();
}

renderer::model renderer::setup_square(glm::vec3 clr)
{
	model mdl;
	std::array<glm::vec2, 4> pos_vtx{ // vertex position attributes
	glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
	glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};

	std::array<glm::vec3, 4> clr_vtx{
	clr, clr,
	clr,clr
	};

	std::array<glm::vec2, 4> texpos{
	glm::vec2(1.f, 0.f), glm::vec2(1.f,1.f),
	glm::vec2(0.f, 1.f), glm::vec2(0.f, 0.f)
	};

	
	GLsizei position_data_offset = 0;
	GLsizei position_attribute_size = sizeof(glm::vec2);
	GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(pos_vtx.size());
	GLsizei color_data_offset = position_data_size;
	GLsizei color_attribute_size = sizeof(glm::vec3);
	GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(clr_vtx.size());
	GLsizei texture_data_offset = position_data_size + color_data_size;
	GLsizei texture_attibute_size = sizeof(glm::vec2);
	GLsizei texture_data_size = texture_attibute_size * static_cast<GLsizei>(texpos.size());

	glCreateBuffers(1, &mdl.vbo);
	glNamedBufferStorage(mdl.vbo, position_data_size + color_data_size + texture_data_size, nullptr, GL_DYNAMIC_STORAGE_BIT);                     // immutable
	glNamedBufferSubData(mdl.vbo, 0, position_data_size, pos_vtx.data());   // upload positions
	glNamedBufferSubData(mdl.vbo, color_data_offset, color_data_size, clr_vtx.data());   // upload colors
	glNamedBufferSubData(mdl.vbo, texture_data_offset, texture_data_size, texpos.data());   // upload colors

	glCreateVertexArrays(1, &mdl.vaoid);
	glEnableVertexArrayAttrib(mdl.vaoid, 0);
	glVertexArrayVertexBuffer(mdl.vaoid, 0, mdl.vbo, position_data_offset, position_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 0, 0);

	glEnableVertexArrayAttrib(mdl.vaoid, 1);
	glVertexArrayVertexBuffer(mdl.vaoid, 1, mdl.vbo, color_data_offset, color_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 1, 1);

	glEnableVertexArrayAttrib(mdl.vaoid, 2);
	glVertexArrayVertexBuffer(mdl.vaoid, 2, mdl.vbo, texture_data_offset, texture_data_size);
	glVertexArrayAttribFormat(mdl.vaoid, 2, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 2, 2);

	mdl.primitive_type = GL_TRIANGLES;
	std::array<GLushort, 6> idx_vtx{ 0, 1, 2, 2, 3, 0 };
	mdl.elem_cnt = static_cast<GLuint>(idx_vtx.size());
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort) * mdl.elem_cnt, reinterpret_cast<GLvoid*>(idx_vtx.data()), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(mdl.vaoid, ebo_hdl);
	mdl.ebo = ebo_hdl;
	glBindVertexArray(0);
	mdl.draw_cnt = static_cast<GLsizei>(pos_vtx.size());
	mdl.primitive_cnt = mdl.elem_cnt;
	mdl.shape = shape::square;
	return mdl;
}

renderer::model renderer::setup_circle(glm::vec3 clr, int slices)
{
	std::vector<glm::vec2> pos_vtx;
	std::vector<glm::vec3> clr_vtx;
	pos_vtx.reserve(slices + 2);
	clr_vtx.reserve(slices + 2);
	pos_vtx.emplace_back(0, 0);
	clr_vtx.emplace_back(clr);
	for (int i = 0; i < slices + 1; ++i)
	{
		float theta = glm::radians(360.0f * i / slices);
		pos_vtx.emplace_back(cos(theta), sin(theta));
		if (i == slices)
		{
			clr_vtx.emplace_back(clr_vtx[1]);
		}
		else
		{
			clr_vtx.emplace_back(clr.r, clr.g, clr.b);

		}
	}
	model mdl;
	GLsizei pos_size = static_cast<GLsizei>(sizeof(glm::vec2) * pos_vtx.size());
	GLsizei clr_size = static_cast<GLsizei>(sizeof(glm::vec3) * clr_vtx.size());
	// set up VAO as in GLApp::points_model
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, pos_size + clr_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo_hdl, 0, pos_size, pos_vtx.data());
	glNamedBufferSubData(vbo_hdl, pos_size, clr_size, clr_vtx.data());

	glCreateVertexArrays(1, &mdl.vaoid);
	glEnableVertexArrayAttrib(mdl.vaoid, 0);
	glVertexArrayVertexBuffer(mdl.vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(mdl.vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 0, 0);
	//gernerate color
	glEnableVertexArrayAttrib(mdl.vaoid, 1);
	glVertexArrayVertexBuffer(mdl.vaoid, 1, vbo_hdl, pos_size, sizeof(glm::vec3));
	glVertexArrayAttribFormat(mdl.vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 1, 1);
	glBindVertexArray(0);
	// and then set up the rest of object mdl ...
	mdl.primitive_type = GL_TRIANGLE_FAN;
	mdl.draw_cnt = static_cast<GLuint>(pos_vtx.size()); // number of vertices
	mdl.elem_cnt = 0;       // no EBO
	mdl.ebo = 0;
	mdl.primitive_cnt = slices; // number of primitives
	mdl.shape = shape::circle;
	return mdl;
}

void renderer::setup_shdrpgm()//const glm::mat4& p_modelMat)
{
	shdr_pgm = LoadShaders("shaders/funny.vert", "shaders/funny.frag", true);
	/*for (model &mdl : models)
	{
		mdl.shdr_pgm = LoadShaders("shaders/funny.vert", "shaders/funny.frag", true);
		if (mdl.shdr_pgm == 0)
			fprintf(stderr, "Shader program failed to link/load.\n");
	}*/
}

//void renderer::draw()
//{
//	glUseProgram(shdr_pgm);
//	GLint vTransformView = glGetUniformLocation(shdr_pgm, "V");
//	glUniformMatrix4fv(vTransformView, 1, GL_FALSE, glm::value_ptr(cam.view));
//	GLint vTransformProj = glGetUniformLocation(shdr_pgm, "P");
//	glUniformMatrix4fv(vTransformProj, 1, GL_FALSE, glm::value_ptr(cam.proj));
//	for (std::map<std::string, object>::iterator i = objects.begin(); i != objects.end(); ++i)
//	{
//		(*i).second.update();
//		(*i).second.draw();
//	}
//}

//void renderer::update()
//{
//	cam.update();
//	draw();
//	
//}

//void renderer::object::init()
//{
//	scaling = glm::vec3(1, 1, 1);
//	rot = 0;
//	translate = glm::vec3(5, 0, 0);
//}

//void renderer::object::update()
//{
//	float x{}, y{}, rot{};
//	//if(keyinput == 'A') x = -2;
//	//if(keyinput == 'D') x = 2;
//	//if(keyinput == 'W') y = 2;
//	//if(keyinput == 'S') y = -2;
//
//	
//	movex(x);
//	movey(y);
//	rotate(rot);
//	float angle = glm::radians(rot);
//	mdl_world = glm::mat4(1.f);
//	mdl_world = glm::scale(mdl_world, scaling);
//	mdl_world = glm::rotate(mdl_world, angle, glm::vec3(0, 0, 1.f));
//	mdl_world = glm::translate(mdl_world, translate);
//}



//void renderer::object::draw()
//{
//	
//	glBindVertexArray(model_ref.vaoid);
//	GLint vTransformLoc = glGetUniformLocation(shdr_pgm, "M");
//	glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, glm::value_ptr(mdl_world));
//
//	//std::cout << glm::to_string(objects.find("player")->second.mdl_world) << std::endl;
//
//	//GLint vTransformLoc = glGetUniformLocation(mdl.shdr_pgm, "vertexTransform");
//	//glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &p_modelMat[0][0]);
//	if (model_ref.shape == shape::square)
//		//glDrawElements(model_ref.primitive_type, model_ref.primitive_cnt, GL_UNSIGNED_SHORT, nullptr);
//		glDrawElements(GL_TRIANGLES, model_ref.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
//	else if (model_ref.shape == shape::circle)
//		glDrawArrays(model_ref.primitive_type, 0, model_ref.draw_cnt);
//	glBindVertexArray(0);
//}

//void renderer::object::movex(float x)
//{
//	translate.x += x;
//}
//
//void renderer::object::movey(float y)
//{
//	translate.y += y;
//}
//
//void renderer::object::rotate(float thetha)
//{
//	rot += thetha;
//}

void renderer::camera::init(int w, int h)
{
	ar = static_cast<float>(w) / h;
	width = w;
	rot = 0;
	zoom = 20;
	campos = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);

	
}

void renderer::camera::update()
{
	view = glm::lookAt(campos, glm::vec3(0, 0, -1), up);
	proj = glm::ortho(-zoom * ar, zoom * ar, -zoom, zoom, -1.f, 1.f);
}



