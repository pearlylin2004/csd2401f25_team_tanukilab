#include "renderer.h"

std::vector<renderer::model> renderer::models;
void renderer::init()
{
	models.push_back(setup_square(glm::vec3(1.f, 1.f, 1.f)));
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

	
	GLsizei position_data_offset = 0;
	GLsizei position_attribute_size = sizeof(glm::vec2);
	GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(pos_vtx.size());
	GLsizei color_data_offset = position_data_size;
	GLsizei color_attribute_size = sizeof(glm::vec3);
	GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(clr_vtx.size());

	
	glCreateBuffers(1, &mdl.vbo);
	glNamedBufferStorage(mdl.vbo, position_data_size + color_data_size, nullptr, GL_DYNAMIC_STORAGE_BIT);                     // immutable
	glNamedBufferSubData(mdl.vbo, 0, position_data_size, pos_vtx.data());   // upload positions
	glNamedBufferSubData(mdl.vbo, color_data_offset, color_data_size, clr_vtx.data());   // upload colors

	glCreateVertexArrays(1, &mdl.vaoid);
	glEnableVertexArrayAttrib(mdl.vaoid, 0);
	glVertexArrayVertexBuffer(mdl.vaoid, 0, mdl.vbo, position_data_offset, position_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 0, 0);

	glEnableVertexArrayAttrib(mdl.vaoid, 1);
	glVertexArrayVertexBuffer(mdl.vaoid, 1, mdl.vbo, color_data_offset, color_attribute_size);
	glVertexArrayAttribFormat(mdl.vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(mdl.vaoid, 1, 1);
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
	return mdl;
}

void renderer::setup_shdrpgm()//const glm::mat4& p_modelMat)
{
	for (model &mdl : models)
	{
		mdl.shdr_pgm = LoadShaders("shaders/funny.vert", "shaders/funny.frag", true);
		if (mdl.shdr_pgm == 0)
			fprintf(stderr, "Shader program failed to link/load.\n");
	}
}

void renderer::draw()
{
	for (model &mdl : models)
	{
		glUseProgram(mdl.shdr_pgm);
		glBindVertexArray(mdl.vaoid);
		//GLint vTransformLoc = glGetUniformLocation(mdl.shdr_pgm, "vertexTransform");
		//glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &p_modelMat[0][0]);
		glDrawElements(mdl.primitive_type, mdl.elem_cnt, GL_UNSIGNED_SHORT, nullptr);
		glBindVertexArray(0);
	}
}

void renderer::update(GLFWwindow* window)
{
	//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//std::string title{ "testing" };
	//glfwSetWindowTitle(GLHelper::ptr_window, title.c_str());
	draw();
	//glfwSwapBuffers(window);
}
