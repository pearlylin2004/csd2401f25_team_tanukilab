/* Start Header ************************************************************************/
/*!
\file		CoreEngine.h
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1, 2025
\brief		Declaration for the CoreEngine class.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <GL/glew.h>
#include <stdexcept>
#include <iostream>
#include "config.h"
#include "debug.h"
#include "input.h"
#include "audio.h"
#include "Systems.h"
#include "luaSystem.h"
#include "ResourceManager.h"
#include "layerManager.h"
#include "prefabManager.h"
#include "GUISystem.h"
#include "messageBus.h"
#include "JsonIO.h"
#include "globalRegistry.h"
#include "GameScript.h"
#include "PathManager.h"
#include "SceneManager.h"

// @class CoreEngine
// @brief Central class for managing the entire game engine lifecycle.
//
// CoreEngine handles initialization, the main game loop, window and input callbacks,
// scene management, Lua scripting, and interaction with all core subsystems (Physics,
// Rendering, Audio, UI, etc.). It acts as the central hub connecting GameObjects,
// GameScripts, and engine systems.
//
// The engine supports pause/resume (alt-tab), fullscreen/windowed mode toggle,
// and allows binding custom update functions or launcher functions.
class CoreEngine {
public:
    // @brief Constructor initializes CoreEngine state.
    CoreEngine();

    // @brief Destructor shuts down CoreEngine and cleans up resources.
    ~CoreEngine();

    // Initializes all engine systems and the window
    void Init(bool forceWindowed = false);

    // Runs the main game loop
    void Run();

    // Shuts down all engine systems
    void Shutdown();

    //for resume and pause
    static void windowIconifyCallback(GLFWwindow* window, int iconified);
    static void windowFocusCallback(GLFWwindow* window, int focused);

    // @brief Sets the current GameScript to handle game logic.
    // @param script Pointer to a GameScript instance.
    void setGameScript(GameScript* script);

    // @brief Returns the current GameObjectManager instance.
    // @return Reference to GameObjectManager.
    GameObjectManager& getManager() { return *m_manager; }

    // @brief Returns the LuaSystem used by this engine.
    // @return Reference to LuaSystem.
    LuaSystem& getLuaSystem() { return *m_luaSystem; }

    // @brief Returns the GLFW window handle.
    // @return Pointer to GLFWwindow.
    GLFWwindow* getWindow() const { return m_window; }

    // @brief Sets a launcher function that can be executed before the game loop.
    // @param fn Launcher function that returns true to continue or false to cancel.
    void setLauncher(std::function<bool()> fn) { m_launcherFn = fn; }
    
    // @brief Loads an initial scene by name.
    // @param sceneName Name of the scene to load.
    void loadInitialScene(const std::string& sceneName);

    // @brief Sets a callback to be executed every update tick.
    // @param fn Function to execute on update.
    void setOnUpdate(std::function<void()> fn) { m_onUpdate = fn; }

    // @brief Stops the main game loop and exits Run().
    void stop() { m_isRunning = false; }

private:
    // @brief Main internal game loop; handles input, update, physics and rendering.
    void GameLoop();

    // @brief Updates all systems for a single frame.
    // @param deltaTime Time elapsed since last frame.
    void Update(float deltaTime);
    double m_fixedDt = 1.0 / 60.0; // 60 Hz simulation

    GLFWwindow* m_window;
    double m_fps;
    double m_delta;
    std::string m_title;
    bool m_isRunning;

    bool m_isFullscreen = false;
    int  m_windowedWidth = 0;
    int  m_windowedHeight = 0;
    int  m_windowPosX = 100;
    int  m_windowPosY = 100;
    bool m_forceWindowed = false;
	bool m_isPaused; //for alt-tab pause

    
    // @brief Toggles between fullscreen and windowed mode.
    void toggleFullscreen();

    GameScript* m_gameScript = nullptr;

    // Core Engine Systems
    std::unique_ptr<GameObjectManager> m_manager;
    std::unique_ptr<InputSystem> m_inputSystem;
    std::unique_ptr<CollisionSystem> m_collisionSystem;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<RenderSystem> m_renderSystem;
    std::unique_ptr<UISystem> m_uiSystem;
    std::unique_ptr<LuaSystem> m_luaSystem;
    std::unique_ptr<FontSystem> m_fontSystem;
    std::unique_ptr<LogicSystem> m_logicSystem;
    std::unique_ptr<MessageBus> m_messageBus;
    std::unique_ptr<PlayerControllerSystem> m_playerController;
    std::unique_ptr<AudioSystem> m_audioSystem;
    std::unique_ptr<TileMapSystem> m_tileMapSystem;
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<VisualLogicSystem> m_visualLCSystem;
    std::unique_ptr<WaterSystem> m_waterSystem;
	std::unique_ptr<TransformSystem> m_transformSystem;
    std::unique_ptr<Camera> m_cameraSystem;

    std::function<bool()> m_launcherFn;
    std::function<void()> m_onUpdate = nullptr;
};
