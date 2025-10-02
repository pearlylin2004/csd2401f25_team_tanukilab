/* Start Header ************************************************************************/
/*!
\file		CoreEngine.cpp
\author
\par
\date		October 1, 2025
\brief		Implementation for the main engine class.
*/
/* End Header **************************************************************************/

#define STB_IMAGE_IMPLEMENTATION
#include "CoreEngine.h"

CoreEngine::CoreEngine()
    : m_window(nullptr), m_fps(0.0), m_delta(0.0), m_isRunning(false), audio(AudioHandler::getInstance()) {
}

CoreEngine::~CoreEngine() {
    // The smart pointers will automatically clean up the systems
}

void CoreEngine::Init() {
    AppConfig cfg;
    std::string err;
    if (!LoadConfig("../projects/my-gl-app/resources/config.txt", cfg, &err)) {
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
    InputHandler::init(m_window);
    renderer::init(cfg.width, cfg.height);

    m_manager = std::make_unique<GameObjectManager>();
    m_inputSystem = std::make_unique<InputSystem>();
    m_collisionSystem = std::make_unique<CollisionSystem>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();
    m_renderSystem = std::make_unique<RenderSystem>();
    m_uiSystem = std::make_unique<UISystem>();

    m_manager->init();
	m_renderSystem->init(*m_manager);

    // --- ImGui Setup ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; //docking
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    audio.init();
    audio.playSound(soundID::bg, 0.1f);

    m_isRunning = true;
}

void CoreEngine::Run() {
    while (m_isRunning) {
        GameLoop();
    }
}

void CoreEngine::GameLoop() {
    if (glfwWindowShouldClose(m_window) || InputHandler::isKeyHeld(GLFW_KEY_ESCAPE)) {
        m_isRunning = false;
        return;
    }

    // --- Update Timing ---
    Perf::UpdateTime(m_delta, m_fps, 0.5);
    Perf::UpdateWindowTitle(m_window, m_title, m_fps, true);

    audio.update();

    // --- Main Update ---
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Update(static_cast<float>(m_delta));

    // --- Swapping Buffers ---
    glfwSwapBuffers(m_window);
}

void CoreEngine::Update(float deltaTime) {
    InputHandler::update();

    // Press F9 to simulate a failure (for crash logger)
    if (InputHandler::isKeyTriggered(GLFW_KEY_F9)) {
        throw std::runtime_error("F9 test: simulated failure in update()");
    }

    if (InputHandler::isKeyTriggered(GLFW_KEY_P)) {
        audio.toggleSoundPause(soundID::bg);
    }
    if (InputHandler::isMouseLeftClicked()) {
        audio.playSound(soundID::effect);
    }

    //imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Update all systems in order
    m_inputSystem->update(*m_manager, deltaTime);
    m_collisionSystem->update(*m_manager, deltaTime);
    m_physicsSystem->update(*m_manager, deltaTime);
    m_renderSystem->update(*m_manager, deltaTime);
    m_uiSystem->update(*m_manager, deltaTime);

    //render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CoreEngine::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
