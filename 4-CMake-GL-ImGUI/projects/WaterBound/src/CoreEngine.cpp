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

CoreEngine::CoreEngine()
    : m_window(nullptr), m_fps(0.0), m_delta(0.0), m_isRunning(false), audio(AudioHandler::getInstance()) {
}

CoreEngine::~CoreEngine() {
    // The smart pointers will automatically clean up the systems
}

void CoreEngine::Init() {
    AppConfig cfg;
    std::string err;
    if (!LoadConfig("../projects/WaterBound/resources/config.json", cfg, &err)) {
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

    m_luaSystem->init();
    m_manager->init();
    m_renderSystem->init(*m_manager, cfg.width, cfg.height);
    m_uiSystem->init(m_window, m_renderSystem.get());

    //iterate over all Lua scripts in the folder
    for (const auto& file : std::filesystem::directory_iterator("../projects/WaterBound/scripting")) {
        if (file.path().extension() != ".lua") continue; //have to be .lua files if not it will skip
        std::string filename = file.path().filename().string();

        //strip ".lua" to get object name
        std::string objectName = filename.substr(0, filename.size() - 4);

        GameObject* obj = m_manager->getGameObject(objectName);
        if (obj) {
			obj->addComponent<LuaScript>(file.path().string()); //add LuaScript component to object
            //load Lua script into Lua state
            if (!m_luaSystem->loadScriptForObject(file.path().string(), objectName)) {
                std::cerr << "[Lua] Failed to load script for " << objectName << std::endl;
            }
            else {
                std::cout << "[Lua] Loaded script " << filename << " for object " << objectName << std::endl;
            }
        }
    }

    audio.init();
    glfwPollEvents();
    m_isRunning = true;
}

void CoreEngine::Run() {
    while (m_isRunning) {
        GameLoop();
    }
}

//void CoreEngine::GameLoop() {
//    glfwPollEvents();
//    if (glfwWindowShouldClose(m_window) || InputHandler::isKeyHeld(GLFW_KEY_ESCAPE)) {
//        m_isRunning = false;
//        return;
//    }
//    // --- Update Timing ---
//
//    int    steps = 0;        // how many fixed updates to run this frame
//    double alpha = 0.0;      // interpolation factor [0,1)
//    //Perf::UpdateTime(m_delta, m_fps, 0.5);
//    Perf::UpdateTime(m_delta, m_fps, 0.5,
//        m_fixedDt, steps, alpha,8);
//
//    Perf::UpdateWindowTitle(m_window, m_title, m_fps, true);
//
//    audio.update();
//
//    InputHandler::update();
//
//
//    for (int i = 0; i < steps; ++i) {
//
//        m_collisionSystem->update(*m_manager, static_cast<float>(m_fixedDt));
//        m_physicsSystem->update(*m_manager, static_cast<float>(m_fixedDt));
//   
//        // (put any other time-based deterministic systems here)
//    }
//    // --- Main Update ---
//    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    m_inputSystem->update(*m_manager, (float)m_delta);
//    m_renderSystem->update(*m_manager, (float)m_delta);
//    m_uiSystem->update(*m_manager);
//
//
//    // --- Swapping Buffers ---
//    glfwSwapBuffers(m_window);
//
//}

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

	//for clonigng player2
    if (InputHandler::isKeyTriggered(GLFW_KEY_C)) {
        static int cloneCount = 1; // Use a static counter for unique names
        std::string newName = "player2_clone_" + std::to_string(cloneCount++);
        GameObject* newObject = m_manager->CloneGameObject("player2", newName);

        std::cout << newName << " created.\n";

        Transform* newTransform = newObject->getComponent<Transform>();
        newTransform->y += 1.5f * (static_cast<float>(cloneCount) - 1); // Offset the new clone's position
    }

    //lua scripting update for all objects with LuaScript component
    std::vector<GameObject*> gameObjects;
    m_manager->getAllGameObjects(gameObjects);

    //running Lua scripts for each object that has a LuaScript component
    for (GameObject* obj : gameObjects) {
        if (obj->hasComponent<LuaScript>()) {
            //LuaScript* scriptComp = obj->getComponent<LuaScript>();
            m_luaSystem->updateObjectScript(obj->getObjectName(), obj, deltaTime);
        }
    }

    // Update all systems in order
    m_inputSystem->update(*m_manager, deltaTime);
    m_logicSystem->update(*m_manager, deltaTime);
    m_collisionSystem->update(*m_manager, deltaTime);
    m_physicsSystem->update(*m_manager, deltaTime);
    m_renderSystem->update(*m_manager, deltaTime);
    m_uiSystem->update(*m_manager);

}

void CoreEngine::Shutdown() {
    if (m_luaSystem) m_luaSystem->cleanup();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
