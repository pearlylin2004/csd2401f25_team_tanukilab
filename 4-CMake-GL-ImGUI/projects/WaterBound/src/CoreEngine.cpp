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
#include "CoreEngine.h"
#include "GUISystem.h"  // Add this include

CoreEngine::CoreEngine()
    : m_window(nullptr), m_fps(0.0), m_delta(0.0), m_isRunning(false) {
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
    // --- Set the user pointer BEFORE setting the callback ---
    glfwSetWindowUserPointer(m_window, this);

    // --- Register the iconify callback ---
    glfwSetWindowIconifyCallback(m_window, CoreEngine::windowIconifyCallback);

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
    AudioHandler::getInstance().init();

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
    m_audioSystem = std::make_unique<AudioSystem>();

    PrefabManager::Instance().loadPrefabRegistry();
    m_guiSystem->init(*m_manager, *m_luaSystem);
    m_luaSystem->init();
	m_renderSystem->init(*m_manager, cfg.width, cfg.height);
	m_uiSystem->init(m_window, m_renderSystem.get());
    m_fontSystem->init(*m_manager);
    m_audioSystem->init(*m_manager);

    //advance messaging 
    m_messageBus->subscribe("KeyPressed", m_playerController.get());
    m_messageBus->subscribe("KeyReleased", m_playerController.get());

    glfwPollEvents();
    m_isRunning = true;

    //AudioHandler::getInstance().playSound(soundID::bg, 0.2f);
}

void CoreEngine::Run() {
    while (m_isRunning) {
        GameLoop();
    }
}

void CoreEngine::GameLoop() {
    if (glfwWindowShouldClose(m_window) || InputHandler::isKeyHeld(GLFW_KEY_ESCAPE)) { m_isRunning = false; return; } 
   
    //use glfwGetWindowAttrib to check the GLFW_ICONIFIED status for efficient waiting
    int status = glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);

    if (status) {
        //window is minimized audio is already paused by the callback.
        //use glfwWaitEvents() to sleep until an event occurs (like restoring the window)
        glfwWaitEvents();
        return; //skip the rest of the loop
    }

    //--- Update Timing --- 
     Perf::UpdateTime(m_delta, m_fps, 0.5);
     Perf::UpdateWindowTitle(m_window, m_title, m_fps, true);
     AudioHandler::getInstance().update(m_delta);
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
    m_audioSystem->update(*m_manager, deltaTime);
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

void CoreEngine::windowIconifyCallback(GLFWwindow* window, int iconified) {
    CoreEngine* engine = static_cast<CoreEngine*>(glfwGetWindowUserPointer(window)); //retrieve engine instance
    if (engine) {
        if (iconified) {

            //window was minimized
            //engine->audio.pauseAll();

            AudioHandler::getInstance().pauseAll(*engine->m_manager);
        }
        else {
            //window was restored maximized or simply returned to normal state
            //resume the background music or other necessary sounds
            //engine->audio.playSound(soundID::bg, 0.2f);
            AudioHandler::getInstance().resumeAll(*engine->m_manager);
        }

    }
}
