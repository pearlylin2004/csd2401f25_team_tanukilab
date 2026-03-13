/* Start Header ************************************************************************/
/*!
\file        WaterSystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
             -100%
\par        asha.m@digipen.edu
\date        January, 28, 2026
\brief       Implementation of water system for animated water bodies, collision,
            buoyancy for object

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"
#include "ResourceManager.h"
#include <iostream>
#include <vector>
#include "Component.h"

WaterSystem::WaterSystem()
    : m_waterShaderProgram(0),
    m_timeLocation(-1),
    m_waveAmplitudeLocation(-1),
    m_waveFrequencyLocation(-1),
    m_waveSpeedLocation(-1),
    m_waterAlphaLocation(-1),
    m_waterColorLocation(-1),
    m_useTextureLocation(-1),
    m_viewMatrixLocation(-1),
    m_projMatrixLocation(-1)
{
}

WaterSystem::~WaterSystem() {
    cleanup();
}

void WaterSystem::init() {
    std::cout << "[WaterSystem] Initializing water system..." << std::endl;

    // Load water shader
    m_waterShaderProgram = ResourceManager::getInstance().getShader("shaders/water.vert", "shaders/water.frag");

    if (m_waterShaderProgram == 0) {
        std::cerr << "Failed to load water shader!" << std::endl;
        return;
    }

    std::cout << "[WaterSystem] Water shader program ID: " << m_waterShaderProgram << std::endl;

    // Get uniform locations
    m_timeLocation = glGetUniformLocation(m_waterShaderProgram, "uTime");
    m_waveAmplitudeLocation = glGetUniformLocation(m_waterShaderProgram, "uWaveAmplitude");
    m_waveFrequencyLocation = glGetUniformLocation(m_waterShaderProgram, "uWaveFrequency");
    m_waveSpeedLocation = glGetUniformLocation(m_waterShaderProgram, "uWaveSpeed");
    m_waterAlphaLocation = glGetUniformLocation(m_waterShaderProgram, "uWaterAlpha");
    m_waterColorLocation = glGetUniformLocation(m_waterShaderProgram, "uWaterColor");
    m_useTextureLocation = glGetUniformLocation(m_waterShaderProgram, "uUseTexture");
    m_viewMatrixLocation = glGetUniformLocation(m_waterShaderProgram, "V");
    m_projMatrixLocation = glGetUniformLocation(m_waterShaderProgram, "P");

    std::cout << "[WaterSystem] Water system initialized successfully" << std::endl;
}

void WaterSystem::update(GameObjectManager& manager, float deltaTime) {
    std::vector<GameObject*> allObjects;
    manager.getAllGameObjects(allObjects);

    // Update water animation time
    for (GameObject* obj : allObjects) {
        WaterBody* water = obj->getComponent<WaterBody>();

        if (water) {
            if (water->renderWaves) {
                water->waveTime += deltaTime;
            }

            if (water->pendingHeightIncrease > 0.0f) {
                float rise = water->heightRiseSpeed * deltaTime;
                rise = Math_Min(rise, water->pendingHeightIncrease);

                water->height += rise;
                water->pendingHeightIncrease -= rise;
                water->lastFrameRise = rise;

                Transform* transform = obj->getComponent<Transform>();
                if (transform) {
                    transform->y += rise * 0.5f;
                }
            }
            else {
                water->lastFrameRise = 0.0f;
            }
            if (water && water->renderWaves) {
                if (!EditorManager::isEditingMode() && !EditorManager::isPaused())
                    water->waveTime += deltaTime;
            }
        }
    }

    // Apply water physics to objects
    applyWaterPhysics(manager, deltaTime);
}

void WaterSystem::render(GameObjectManager& manager) {
    if (m_waterShaderProgram == 0) return;

    std::vector<GameObject*> allObjects;
    manager.getAllGameObjects(allObjects);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use water shader
    glUseProgram(m_waterShaderProgram);

    // Set view and projection matrices
    auto& activeCam = EditorManager::isEditingMode() ? renderer::editorCam : renderer::cam;
    glUniformMatrix4fv(m_viewMatrixLocation, 1, GL_FALSE, &activeCam.view[0][0]);
    glUniformMatrix4fv(m_projMatrixLocation, 1, GL_FALSE, &activeCam.proj[0][0]);

    // Render each water body
    for (GameObject* obj : allObjects) {
        WaterBody* water = obj->getComponent<WaterBody>();
        if (water) {
            renderWaterBody(obj);
        }
    }

    glDisable(GL_BLEND);
}

void WaterSystem::setupWaterMesh(GameObject* waterObj) {
    WaterBody* water = waterObj->getComponent<WaterBody>();
    Transform* transform = waterObj->getComponent<Transform>();

    if (!water || !transform) return;

    // Check if mesh already exists
    if (m_waterMeshes.find(waterObj) != m_waterMeshes.end()) {
        return; // Already set up
    }

    WaterMesh mesh;
    mesh.subdivisions = water->subdivisions;

    // Create subdivided quad vertices
    std::vector<glm::vec2> positions;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> texCoords;
    std::vector<GLuint> indices;

    //int numVertices = (mesh.subdivisions + 1) * 2; // Top and bottom rows

    // Generate vertices
    for (int i = 0; i <= mesh.subdivisions; ++i) {
        float t = static_cast<float>(i) / mesh.subdivisions;
        float x = -0.5f + t; // -0.5 to 0.5

        // Top vertex 
        positions.push_back(glm::vec2(x, 0.5f));
        colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
        texCoords.push_back(glm::vec2(t, 1.0f));

        // Bottom vertex
        positions.push_back(glm::vec2(x, -0.5f));
        colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
        texCoords.push_back(glm::vec2(t, 0.0f));
    }

    // Generate indices for triangle strip
    for (int i = 0; i < mesh.subdivisions; ++i) {
        int topLeft = i * 2;
        int bottomLeft = i * 2 + 1;
        int topRight = (i + 1) * 2;
        int bottomRight = (i + 1) * 2 + 1;

        // 1st triangle
        indices.push_back(topLeft);
        indices.push_back(bottomLeft);
        indices.push_back(topRight);

        // 2nd triangle
        indices.push_back(topRight);
        indices.push_back(bottomLeft);
        indices.push_back(bottomRight);
    }

    mesh.elementCount = static_cast<int>(indices.size());

    // Create VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create VBO
    GLsizei posSize = static_cast<GLsizei>(positions.size() * sizeof(glm::vec2));
    GLsizei colSize = static_cast<GLsizei>(colors.size() * sizeof(glm::vec3));
    GLsizei texSize = static_cast<GLsizei>(texCoords.size() * sizeof(glm::vec2));

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + colSize + texSize, nullptr, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, positions.data());
    glBufferSubData(GL_ARRAY_BUFFER, posSize, colSize, colors.data());
    glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize, texSize, texCoords.data());

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(uintptr_t)(posSize));

    // TexCoord attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(uintptr_t)(posSize + colSize));

    // Create EBO
    glGenBuffers(1, &mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Create SSBO for instance data
    glGenBuffers(1, &mesh.ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh.ssbo);

    // Prepare instance data
    renderer::InstanceData instanceData;

    // Build model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(transform->x, transform->y, 0.0f));
    model = glm::rotate(model, glm::radians(transform->rotation), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(water->width, water->height, 1.0f));

    instanceData.model = model;
    instanceData.color = water->waterColor;
    instanceData.texParams = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Full texture

    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(renderer::InstanceData), &instanceData, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_waterMeshes[waterObj] = mesh;

    std::cout << "[WaterSystem] Created water mesh with " << mesh.elementCount << " elements" << std::endl;
}

void WaterSystem::renderWaterBody(GameObject* waterObj) {
    WaterBody* water = waterObj->getComponent<WaterBody>();
    Transform* transform = waterObj->getComponent<Transform>();

    if (!water || !transform) return;

    // Setup mesh if not already done
    if (m_waterMeshes.find(waterObj) == m_waterMeshes.end()) {
        setupWaterMesh(waterObj);
    }

    WaterMesh& mesh = m_waterMeshes[waterObj];

    // Update instance data with current transform
    renderer::InstanceData instanceData;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(transform->x, transform->y, 0.0f));
    model = glm::rotate(model, glm::radians(transform->rotation), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(water->width, water->height, 1.0f));

    instanceData.model = model;
    instanceData.color = water->waterColor;
    instanceData.texParams = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    // Update SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh.ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(renderer::InstanceData), &instanceData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.ssbo);

    // Set water uniforms
    glUniform1f(m_timeLocation, water->waveTime);
    glUniform1f(m_waveAmplitudeLocation, water->waveAmplitude);
    glUniform1f(m_waveFrequencyLocation, water->waveFrequency);
    glUniform1f(m_waveSpeedLocation, water->waveSpeed);
    glUniform1f(m_waterAlphaLocation, water->waterAlpha);
    glUniform4fv(m_waterColorLocation, 1, &water->waterColor[0]);
    glUniform1i(m_useTextureLocation, water->useTexture ? 1 : 0);

    // Bind texture if used
    if (water->useTexture && water->waterTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, water->waterTexture);
    }
        
    // Draw
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void WaterSystem::applyWaterPhysics(GameObjectManager& manager, float deltaTime) {
    (void)deltaTime;
    std::vector<GameObject*> allObjects;
    manager.getAllGameObjects(allObjects);

    std::vector<GameObject*> waterBodies;
    for (GameObject* obj : allObjects) {
        if (obj->hasComponent<WaterBody>()) {
            waterBodies.push_back(obj);
        }
    }

    for (GameObject* obj : allObjects) {
        Physics* physics = obj->getComponent<Physics>();
        Transform* transform = obj->getComponent<Transform>();

        if (!physics || !transform) continue;
        if (obj->hasComponent<WaterBody>()) continue;

        
        CollisionInfo* col = obj->getComponent<CollisionInfo>();
        bool isFloatingObject = col && (col->collisionTag == "floatingObject");
        if (EditorManager::isEditingMode() || EditorManager::isPaused()) {
            physics->velY = 0.f;
            physics->velX = 0.f;
            physics->dynamics.velocity.y = 0.f;
            physics->dynamics.velocity.x = 0.f;

            continue;
        }

        physics->inWater = false;


        if (isFloatingObject) {
            for (GameObject* waterObj : waterBodies) {
                WaterBody* water = waterObj->getComponent<WaterBody>();
                Transform* waterTransform = waterObj->getComponent<Transform>();
                if (!water || !waterTransform) continue;


                float waterHalfW = water->width * 0.5f;
                float waterHalfH = water->height * 0.5f;
                float waterMinX = waterTransform->x - waterHalfW;
                float waterMaxX = waterTransform->x + waterHalfW;
                float waterMinY = waterTransform->y - waterHalfH;
                float waterMaxY = waterTransform->y + waterHalfH;

                bool insideWater = (transform->x >= waterMinX && transform->x <= waterMaxX &&
                    transform->y >= waterMinY && transform->y <= waterMaxY);

                if (!insideWater && !physics->inWater) continue;

                physics->inWater = true;

                float waterSurfaceY = waterTransform->y + waterHalfH;

                if (water->lastFrameRise > 0.0f) {
                    transform->y += water->lastFrameRise;
                    physics->velY = 0.0f;
                    physics->dynamics.position.y = transform->y;
                    break;
                }

                float depth = waterSurfaceY - transform->y;
                float halfObjHeight = transform->scaleY * 0.5f;
                float submergedFraction = glm::clamp(depth / (halfObjHeight * 2.0f + 0.001f), 0.0f, 1.0f);

                float safeMass = physics->dynamics.mass;
                float buoyancyAccel = (water->buoyancyForce / safeMass) * submergedFraction * 0.016f * 16.0f;

                physics->velY += buoyancyAccel;


                physics->velY = glm::clamp(physics->velY, physics->maxFallSpeed, 8.0f);

                /*float strongDrag = 0.999f;*/
                float verticalDrag = glm::mix(water->waterDrag * 0.999f, water->waterDrag, submergedFraction);
                physics->velY *= verticalDrag;
                physics->velX *= water->waterDrag;

                break;
            }
        }
    }
}
void WaterSystem::cleanup() {

    // Delete all water meshes
    for (auto& pair : m_waterMeshes) {
        WaterMesh& mesh = pair.second;
        if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
        if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
        if (mesh.ebo) glDeleteBuffers(1, &mesh.ebo);
        if (mesh.ssbo) glDeleteBuffers(1, &mesh.ssbo);
    }
    m_waterMeshes.clear();

    if (m_waterShaderProgram) {
        glDeleteProgram(m_waterShaderProgram);
        m_waterShaderProgram = 0;
    }

}