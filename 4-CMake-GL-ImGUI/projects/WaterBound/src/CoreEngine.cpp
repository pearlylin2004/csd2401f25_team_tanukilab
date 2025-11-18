/* Start Header ************************************************************************/
/*!
\file		CoreEngine.cpp
\author 	Seow Sin Le, s.sinle, 2401084, 60%
\author     Hugo Low Ren Hao, low.h, 2402272, 20%
\author     Tan Chee Qing, cheeqing.tan, 2401486, 10%
\author     Varick Teo, v.teo, 2403417, 10%
\author     Pearly Lin Lee Ying, p.lin, 2401591, 5%
\par        s.sinle@digipen.edu
\par        low.h@digipen.edu
\par        cheeqing.tan@digipen.edu
\par        v.teo@digipen.edu
\par        p.lin@digipen.edu
\date		October, 1, 2025
\brief		Implementation for the CoreEngine class. The CoreEngine is responsible for
            Initialisation and Shutdown, Game Loop Execution and System Updates

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#define STB_IMAGE_IMPLEMENTATION
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#include "CoreEngine.h"
#include "GUISystem.h"  // Add this include

CoreEngine::CoreEngine()
    : m_window(nullptr), m_fps(0.0), m_delta(0.0), m_isRunning(false), audio(AudioHandler::getInstance()) {
}

CoreEngine::~CoreEngine() {
    // The smart pointers will automatically clean up the systems
}

void CoreEngine::Init() {
    AppConfig cfg;
    std::string err;
    if (!LoadConfig(std::string(RUNTIME_RES_DIR_R) + "/config.json", cfg, &err)) {
        std::cout << err << "\n";
    }

    m_title = cfg.title;

    // --- GLFW/GLEW Initialization ---
    if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_window = glfwCreateWindow(cfg.width, cfg.height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Window creation failed");
    }
    glfwMakeContextCurrent(m_window);
    glViewport(0, 0, cfg.width, cfg.height);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error("GLEW initialization failed");
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSwapInterval(cfg.vsync ? 1 : 0);
    glClearColor(cfg.clear_color[0], cfg.clear_color[1], cfg.clear_color[2], cfg.clear_color[3]);

    // --- System Initialization ---
    audio.init();

    InputHandler::init(m_window, cfg);
    renderer::init(cfg.width, cfg.height);


    m_manager = std::make_unique<GameObjectManager>();
	m_logicSystem = std::make_unique<LogicSystem>();
    m_inputSystem = std::make_unique<InputSystem>();
    m_collisionSystem = std::make_unique<CollisionSystem>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_renderSystem = std::make_unique<RenderSystem>();
    m_uiSystem = std::make_unique<UISystem>();
    m_luaSystem = std::make_unique<LuaSystem>();
    m_fontSystem = std::make_unique<FontSystem>();
    m_guiSystem = std::make_unique<GUISystem>();
    m_messageBus = std::make_unique<MessageBus>();
    m_playerController = std::make_unique<PlayerControllerSystem>();

    PrefabManager::Instance().loadPrefabRegistry();
    m_guiSystem->init(*m_manager, *m_luaSystem);
    m_luaSystem->init();
	m_renderSystem->init(*m_manager, cfg.width, cfg.height);
	m_uiSystem->init(m_window, m_renderSystem.get());
    m_fontSystem->init(*m_manager);

    //advance messaging 
    m_messageBus->subscribe("KeyPressed", m_playerController.get());
    m_messageBus->subscribe("KeyReleased", m_playerController.get());

    glfwPollEvents();
    m_isRunning = true;

    AudioHandler::getInstance().playSound(soundID::bg, 0.2f);
}

void CoreEngine::Run() {
    while (m_isRunning) {
        GameLoop();
    }
}

void CoreEngine::GameLoop() {
    if (glfwWindowShouldClose(m_window) || InputHandler::isKeyHeld(GLFW_KEY_ESCAPE)) { m_isRunning = false; return; } 
     //--- Update Timing --- 
     Perf::UpdateTime(m_delta, m_fps, 0.5);
     Perf::UpdateWindowTitle(m_window, m_title, m_fps, true);
     audio.update();
     // --- Main Update --- glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     Update(static_cast<float>(m_delta));
     // --- Swapping Buffers --- 
     glfwSwapBuffers(m_window); }

void CoreEngine::Update(float deltaTime) {
    InputHandler::update();
    m_guiSystem->update(*m_manager);

    if(!EditorManager::isEditingMode() && !EditorManager::isPaused()) m_luaSystem->update(*m_manager, deltaTime);

    // Update all systems in order
    m_inputSystem->update(*m_manager, deltaTime, *m_messageBus);
    m_physicsSystem->update(*m_manager, deltaTime, *m_messageBus);
    m_collisionSystem->update(*m_manager, deltaTime);
    m_logicSystem->update(*m_manager, deltaTime);
    m_renderSystem->update(*m_manager, deltaTime);
    m_fontSystem->update(*m_manager);
    m_uiSystem->update(*m_manager);

    //performance update
    LogSystemTimersEveryInterval(deltaTime,15.0);
}

void CoreEngine::Shutdown() {
	ResourceManager::getInstance().shutdown();
    renderer::cleanup();
    Font::freeFonts();
    if (m_luaSystem) m_luaSystem->cleanup();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

//---------- WINDOWS ----------
//when windows lost focus
void CoreEngine::onWindowFocusGained() {
    m_windowHasFocus = true;

    //resume only if window is not minimized
    if (!m_isMinimized && m_isPaused) {
        m_isPaused = false;
        audio.resumeAll();
        InputHandler::update();
        //InputHandler::clearState();
        //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        std::cout << "Resumed all audio due to focus gained" << std::endl;
    }
}

//when windows gained focus
void CoreEngine::onWindowFocusLost() {
    m_windowHasFocus = false;

    if (!m_isPaused) {
        m_isPaused = true;
        audio.pauseAll();
        InputHandler::clearState();
        //glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        std::cout << "Paused all audio due to focus lost" << std::endl;
    }
}
