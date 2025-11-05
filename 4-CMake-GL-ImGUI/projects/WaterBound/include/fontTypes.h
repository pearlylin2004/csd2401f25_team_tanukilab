#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

struct FontCharacter {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};