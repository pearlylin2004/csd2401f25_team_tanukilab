#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <glm/glm.hpp>
#include <map>
#include <shader.hpp>

struct Font
{
    struct FontMdl
    {
        GLuint vao, vbo;
    };
    struct Character {
        GLuint TextureID;  // ID handle of the glyph texture
        glm::ivec2   Size;       // Size of glyph
        glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
        GLuint Advance;    // Offset to advance to next glyph
        FontMdl mesh;
    };

    static std::vector<FontMdl> fontMdls;
    static std::vector<FT_Face> face;
    static FT_Library ft;
    static int fontTypes;
    static std::map<char, Character> Characters;
	static int init();
    static int update(size_t fontElement);
    static FontMdl fontMeshInit();
    static void uploadFontTex(size_t fontElement);
    static void freeFonts();
    static GLuint fontShaders;
};