/* Start Header ************************************************************************/
/*!
\file		CoreEngine.h
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October 1, 2025
\brief		Declaration for the main engine class.
*/
/* End Header **************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "config.h"
#include "debug.h"
#include "input.h"
#include "GameObjectManager.h"
#include "Systems.h"

class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine();

    // Initializes all engine systems and the window
    void Init();

    // Runs the main game loop
    void Run();

    // Shuts down all engine systems
    void Shutdown();

private:
    void GameLoop();
    void Update(float deltaTime);

    // Window and timing variables
    GLFWwindow* m_window;
    double m_fps;
    double m_delta;
    std::string m_title;
    bool m_isRunning;

    // Core Engine Systems
    std::unique_ptr<GameObjectManager> m_manager;
    std::unique_ptr<InputSystem> m_inputSystem;
    std::unique_ptr<CollisionSystem> m_collisionSystem;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<RenderSystem> m_renderSystem;
    std::unique_ptr<UISystem> m_uiSystem;
};