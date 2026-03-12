/* Start Header ************************************************************************/
/*!
\file        WaterSystem.h
\author      Asha Mathyalakan, asha.m, 
             2402886 , 100%
\par         asha.m@digipen.edu
\date        January, 28, 2026
\brief       Water system for rendering movement of 
            water bodies and handling water physics

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "GameObjectManager.h"
#include "Component.h"
#include "renderer.h"
#include <GL/glew.h>

class WaterSystem {
public:
    WaterSystem();
    ~WaterSystem();

    // Initialize water system and load water shaders
    void init();

    // Update water animation and physics
    void update(GameObjectManager& manager, float deltaTime);

    // Render all water bodies
    void render(GameObjectManager& manager);


    // Apply water physics to objects
    void applyWaterPhysics(GameObjectManager& manager, float deltaTime);

    // Cleanup
    void cleanup();

private:
    // Setup water mesh for a water body
    void setupWaterMesh(GameObject* waterObj);

    // Render a single water body
    void renderWaterBody(GameObject* waterObj);

    // Water shader program
    GLuint m_waterShaderProgram;

    // Uniform locations
    GLint m_timeLocation;
    GLint m_waveAmplitudeLocation;
    GLint m_waveFrequencyLocation;
    GLint m_waveSpeedLocation;
    GLint m_waterAlphaLocation;
    GLint m_waterColorLocation;
    GLint m_useTextureLocation;
    GLint m_viewMatrixLocation;
    GLint m_projMatrixLocation;

    // Water mesh data
    struct WaterMesh {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint ssbo;
        int elementCount;
        int subdivisions;
    };

    // Cache of water meshes per game object
    std::unordered_map<GameObject*, WaterMesh> m_waterMeshes;
};