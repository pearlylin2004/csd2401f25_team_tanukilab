/* Start Header ************************************************************************/
/*!
\file		CoreEngine.cpp
\author 	Seow Sin Le, s.sinle, 2401084, 55%
\author     Hugo Low Ren Hao, low.h, 2402272, 20%
\author     Tan Chee Qing, cheeqing.tan, 2401486, 10%
\author     Varick Teo, v.teo, 2403417, 10%
\author     Pearly Lin Lee Ying, p.lin, 2401591, 10%
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
#include "LeaderboardSystem.h"
#include "ReplaySystem.h"

CoreEngine::CoreEngine()
    : m_window(nullptr),
    m_fps(0.0),
    m_delta(0.0),
    m_isRunning(false),
    m_isFullscreen(false),
    m_windowedWidth(0),
    m_windowedHeight(0),
    m_windowPosX(100),
    m_windowPosY(100),
    m_forceWindowed(false),
    m_isPaused(false) //for pause
{
}

CoreEngine::~CoreEngine() {
    // The smart pointers will automatically clean up the systems
}

void CoreEngine::Init(bool forceWindowed) {
    AppConfig cfg;
    std::string err;
    if (!LoadConfig(std::string(RUNTIME_RES_DIR_R) + "/config.json", cfg, &err)) {
        std::cout << err << "\n";
    }

    m_title = cfg.title;

    m_forceWindowed = forceWindowed;

    bool wantFullscreen = cfg.fullscreen && !m_forceWindowed;
    m_isFullscreen = wantFullscreen;

    // --- GLFW/GLEW Initialization ---
    if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWmonitor* monitor = nullptr;
    int winWidth = cfg.width;
    int winHeight = cfg.height;

    if (wantFullscreen) {
       

        // remember windowed size for when we toggle back
        m_windowedWidth = cfg.width;
        m_windowedHeight = cfg.height;
        Camera::onResize(m_windowedWidth, m_windowedHeight);
    }
    else {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        winWidth = mode->width;
        winHeight = mode->height;

        m_windowedWidth = cfg.width;
        m_windowedHeight = cfg.height;
		Camera::onResize(winWidth, winHeight);
    }

    cfg.width = winWidth;
    cfg.height = winHeight;

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // set this to true to allow resizing (rmb to change back to false!!)
    m_window = glfwCreateWindow(winWidth, winHeight, m_title.c_str(), monitor, nullptr);

    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Window creation failed");
    }
    // --- Set the user pointer BEFORE setting the callback ---
    glfwSetWindowUserPointer(m_window, this);

    // --- Register the iconify callback ---
    glfwSetWindowIconifyCallback(m_window, CoreEngine::windowIconifyCallback);

    // --- Register the focus callback ---
    glfwSetWindowFocusCallback(m_window, CoreEngine::windowFocusCallback);

    glfwMakeContextCurrent(m_window);
    glViewport(0, 0, winWidth, winHeight);
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
    renderer::init(winWidth, winHeight);

    m_manager = std::make_unique<GameObjectManager>();
    m_logicSystem = std::make_unique<LogicSystem>();
    m_inputSystem = std::make_unique<InputSystem>();
    m_collisionSystem = std::make_unique<CollisionSystem>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_weaponSystem = std::make_unique<WeaponSystem>();
    m_renderSystem = std::make_unique<RenderSystem>();
    m_waterSystem = std::make_unique<WaterSystem>();
#ifdef _DEBUG
    m_uiSystem = std::make_unique<UISystem>();
#endif
    m_luaSystem = std::make_unique<LuaSystem>();
    m_fontSystem = std::make_unique<FontSystem>();
    //m_guiSystem = std::make_unique<GUISystem>();
    m_messageBus = std::make_unique<MessageBus>();
    m_playerController = std::make_unique<PlayerControllerSystem>();
    m_audioSystem = std::make_unique<AudioSystem>();
    m_tileMapSystem = std::make_unique<TileMapSystem>();
	m_particleSystem = std::make_unique<ParticleSystem>();
	m_visualLCSystem = std::make_unique<VisualLogicSystem>();
	m_transformSystem = std::make_unique<TransformSystem>();
	m_cameraSystem = std::make_unique<Camera>();


    PrefabManager::Instance().loadPrefabRegistry();
    //m_guiSystem->init(*m_manager, *m_luaSystem);
    GUISystem::getInstance().init(*m_manager, *m_luaSystem, *m_visualLCSystem);
    m_luaSystem->init();
    m_renderSystem->init(*m_manager, winWidth, winHeight);
    m_waterSystem->init();
#ifdef _DEBUG
    m_uiSystem->init(m_window, m_renderSystem.get(), *m_manager);
#endif
    m_fontSystem->init(*m_manager);
    m_audioSystem->init(*m_manager);

    //hardcoded bgm again
    startBGM();

    //advance messaging 
    m_messageBus->subscribe("KeyPressed", m_playerController.get());
    m_messageBus->subscribe("KeyReleased", m_playerController.get());

    // load global settings
    GlobalRegistry::Instance().init();

    // Initialize leaderboard (loads scores from disk)
    LeaderboardSystem::getInstance().init();

    // Initialize replay system (creates replays directory)
    ReplaySystem::getInstance().init();

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
    //change to fn because Q will affect the editor
    if (glfwWindowShouldClose(m_window) || InputHandler::isKeyTriggered(GLFW_KEY_F1)) { m_isRunning = false; return; }
    //pause checking, and set it to paused state if escape is held
    if (InputHandler::isKeyTriggered(GLFW_KEY_ESCAPE)) {
        auto current = GUISystem::getInstance().getCurrentState(); //get current state
        if (current != GameState::PAUSED && current != GameState::MENU && current != GameState::REPLAY) {
            GUISystem::getInstance().setStateBeforePause(current); //store state before pausing
            GUISystem::getInstance().changeState(GameState::PAUSED, "pause_scene.json");
        }
    }

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
#if defined(_DEBUG) || defined(DEBUG)
    // Show FPS only in debug builds
    Perf::UpdateWindowTitle(m_window, m_title, m_fps, true);
#else
    // Release build never show FPS
    Perf::UpdateWindowTitle(m_window, m_title, 0.0f, false);
#endif
    AudioHandler::getInstance().update(static_cast<float>(m_delta));

    //to skip physics logic lua and input when paused
    if (m_isPaused) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(m_window);
        glfwPollEvents();
        return; //to skip
    }

    // --- Main Update --- glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Update(static_cast<float>(m_delta));
    // --- Swapping Buffers --- 
    glfwSwapBuffers(m_window);
}

void CoreEngine::Update(float deltaTime) {
    //for pausing
    if (m_isPaused) return; //skip update when paused

    InputHandler::update();
    if (InputHandler::isKeyTriggered(GLFW_KEY_F11)) {
        toggleFullscreen();
    }
    //menu checking
    GameState state = GUISystem::getInstance().getCurrentState();
    if (state != GameState::EXIT) {
        GUISystem::getInstance().update();
    }

    // Tick the level timer for leaderboard
    GUISystem::getInstance().tickLevelTimer(deltaTime);

	//for other systems to update even in editor mode
    if (!EditorManager::isEditingMode() && !EditorManager::isPaused()) {
        m_luaSystem->update(*m_manager, deltaTime);
        m_visualLCSystem->initAll(*m_manager);
        m_visualLCSystem->updateAll(*m_manager, deltaTime);
    }

    

    // Update all systems in order
	m_cameraSystem->update(*m_manager);
    renderer::cam.update();
    m_inputSystem->update(*m_manager, deltaTime, *m_messageBus);
    m_physicsSystem->update(*m_manager, deltaTime, *m_messageBus);
    m_collisionSystem->update(*m_manager, deltaTime);
	m_transformSystem->update(*m_manager, deltaTime);//for parent->child transform
    m_weaponSystem->update(*m_manager, deltaTime);
    m_logicSystem->update(*m_manager, deltaTime);
    m_tileMapSystem->update(*m_manager);
    m_particleSystem->update(*m_manager, deltaTime);;
    m_waterSystem->update(*m_manager, deltaTime);

    // Record player state for replay and update ghost playback
    auto& replay = ReplaySystem::getInstance();
    auto currentGS = GUISystem::getInstance().getCurrentState();
    if (replay.isRecording() && currentGS != GameState::PAUSED) {
        replay.recordFrame(*m_manager, deltaTime);
    }
    if (replay.isPlaying() && currentGS != GameState::PAUSED) {
        replay.updateGhost(*m_manager, deltaTime);
    }
    if (replay.isViewerMode() && currentGS != GameState::PAUSED) {
        replay.updateViewer(deltaTime);
    }
    // Apply recorded world object state every frame in viewer mode
    if (replay.isViewerMode()) {
        replay.applyWorldSnapshot(*m_manager);
    }

    m_renderSystem->update(*m_manager, deltaTime);
    m_waterSystem->render(*m_manager);
    m_fontSystem->update(*m_manager, m_fps);
    m_audioSystem->update(*m_manager, deltaTime);
    //m_waterSystem->render(*m_manager);
#ifdef _DEBUG
    m_uiSystem->update();
#endif

    //performance update
    LogSystemTimersEveryInterval(deltaTime, 15.0);

#ifdef _DEBUG
    static bool wasEditingMode = true;
    bool isEditingNow = EditorManager::isEditingMode();

    wasEditingMode = isEditingNow;

    if (wasEditingMode && !isEditingNow) {
        resumeBGM();
    }
    if (!wasEditingMode && isEditingNow) {
        pauseBGM();
    }
#endif
}

// ============================================================================
// Toggle Fullscreen
// ============================================================================

/**
 * @brief Toggles the application between windowed mode and fullscreen mode.
 *
 * This function switches the GLFW window between fullscreen (using the primary
 * monitor's native resolution) and windowed mode (restoring the previous
 * window position and size). After toggling, it updates the OpenGL viewport,
 * input handler, renderer, cameras, and internal FBO sizes to ensure all
 * systems match the new screen dimensions.
 *
 * @note Framebuffer size may differ from window size on high-DPI displays,
 *       so both values are queried separately.
 */
void CoreEngine::toggleFullscreen()
{
    // Toggle fullscreen state flag
    m_isFullscreen = !m_isFullscreen;

    GLFWmonitor* monitor = nullptr;

    if (m_isFullscreen)
    {
        /**
         * -----------------------------
         * WINDOWED to FULLSCREEN
         * -----------------------------
         */

         // Store the current window position and dimensions
        glfwGetWindowPos(m_window, &m_windowPosX, &m_windowPosY);
        glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);

        // Fetch the primary monitor and its current video mode
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Switch the window to fullscreen using monitor resolution
        glfwSetWindowMonitor(
            m_window,
            monitor,
            0, 0,                 ///< Fullscreen always starts at (0,0)
            mode->width,
            mode->height,
            mode->refreshRate
        );
    }
    else
    {
        /**
         * -----------------------------
         * FULLSCREEN to WINDOWED
         * -----------------------------
         */

         // Restore previous window size and position
        glfwSetWindowMonitor(
            m_window,
            nullptr,              ///< nullptr = return to windowed mode
            m_windowPosX,
            m_windowPosY,
            m_windowedWidth,
            m_windowedHeight,
            0                     ///< Refresh rate ignored for windowed mode
        );
    }

    /**
     * --------------------------------------------------------------------
     * UPDATE GL FRAMEBUFFER SIZE & VIEWPORT
     * (Framebuffer size may differ from window size due to DPI scaling.)
     * --------------------------------------------------------------------
     */
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(m_window, &fbw, &fbh);

    // Update OpenGL viewport to new framebuffer resolution
    glViewport(0, 0, fbw, fbh);

    /**
     * --------------------------------------------------------------------
     * UPDATE WINDOW SIZE FOR INPUT SYSTEM
     * (Input uses logical window size, not framebuffer size.)
     * --------------------------------------------------------------------
     */
    int winW = 0, winH = 0;
    glfwGetWindowSize(m_window, &winW, &winH);
    InputHandler::setWindowSize(winW, winH);

    /**
     * --------------------------------------------------------------------
     * NOTIFY RENDERER ABOUT THE RESIZE
     * Recalculates projection matrices or internal buffers.
     * --------------------------------------------------------------------
     */
    renderer::onResize(fbw, fbh);
    Camera::onResize(fbw, fbh);

    // Update both gameplay camera and editor camera to new aspect ratios
    renderer::cam.update();
    renderer::editorCam.update();

    // Process events to ensure GLFW internal states remain in sync
    glfwPollEvents();

    /**
     * --------------------------------------------------------------------
     * RESIZE INTERNAL FRAMEBUFFER OBJECT (FBO)
     * Ensures post-processing / editor rendering matches new window size.
     * --------------------------------------------------------------------
     */
    m_renderSystem->resizeFBO(winW, winH);
}


void CoreEngine::Shutdown() {
    ResourceManager::getInstance().shutdown();
    renderer::cleanup();
    Font::freeFonts();
    m_waterSystem->cleanup();
    if (m_luaSystem) m_luaSystem->cleanup();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

//for minimize and restore
void CoreEngine::windowIconifyCallback(GLFWwindow* window, int iconified) {
    CoreEngine* engine = static_cast<CoreEngine*>(glfwGetWindowUserPointer(window)); //retrieve engine instance
    if (engine) {
        if (iconified) {
            //window was minimized
            //engine->audio.pauseAll();
            AudioHandler::getInstance().pauseAll(*engine->m_manager);
            engine->m_isPaused = true; //set paused flag
            engine->pauseBGM();
        }
        else {
            //window was restored maximized or simply returned to normal state
            //resume the background music or other necessary sounds
            //engine->audio.playSound(soundID::bg, 0.2f);
            if (!EditorManager::isEditingMode()) {
                AudioHandler::getInstance().resumeAll(*engine->m_manager);
                engine->resumeBGM();
            }
            engine->m_isPaused = false; //clear paused flag
        }
    }
}

//for alt-tab --this shld work for ctrl-alt-del too, need to test on lab pcs
void CoreEngine::windowFocusCallback(GLFWwindow* window, int focused) {
    CoreEngine* engine = static_cast<CoreEngine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        if (!focused) {
            engine->m_isPaused = true; //set paused flag
            //window lost focus
            AudioHandler::getInstance().pauseAll(*engine->m_manager);
        }
        else {
            engine->m_isPaused = false; //clear paused flag
            //window regained focus
            if (!EditorManager::isEditingMode()) {
                AudioHandler::getInstance().resumeAll(*engine->m_manager);
            }
        }
    }
}

void CoreEngine::startBGM() {
    if (m_bgmChannel) return;
    AudioChannel bgm;
    bgm.audioFile = m_bgmFilePath;
    bgm.loop = true;
    bgm.volume = 0.125f;
    m_bgmChannel = AudioHandler::getInstance().playBGMSound(&bgm);
    AudioHandler::getInstance().registerBGMChannel(m_bgmChannel);
}

void CoreEngine::stopBGM() {
    if (m_bgmChannel) {
        m_bgmChannel->stop();
        m_bgmChannel = nullptr;
    }
}

void CoreEngine::pauseBGM() {
    if (m_bgmChannel) m_bgmChannel->setPaused(true);
}

void CoreEngine::resumeBGM() {
    if (m_bgmChannel) m_bgmChannel->setPaused(false);
}