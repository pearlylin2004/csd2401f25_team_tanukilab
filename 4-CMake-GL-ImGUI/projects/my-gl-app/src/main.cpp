/* Start Header ************************************************************************/
/*!
\file		main.hpp
\author
\par
\date		Month, Day, 2025
\brief
*/
/* End Header **************************************************************************/

// Hungarian notation: https://en.wikipedia.org/wiki/Hungarian_notation
#include <shader.hpp>
#include <memory>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "renderer.h"
#include "input.h"
#include "audio.h"
#include "collision.h"
#include "debug.h"
#include "ui.h"
#include "GameObjectManager.h"
#include "Component.h"
#include "Systems.h"
#include "config.h"

class GLApp {
public:
    GLApp(int p_width, int p_height, const char* p_title) : m_angleOfRotation(0.0f),
        m_rotationSpeed(0.01f),
        m_window(nullptr),
        m_title(p_title)//, 
        //m_cube(std::make_shared<Cube>())
    {  
        if (!glfwInit()) throw std::runtime_error("GLFW initialization failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        
        m_window = glfwCreateWindow(p_width, p_height, p_title, nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("Window creation failed");
        }
        glfwMakeContextCurrent(m_window);
        glViewport(0, 0, p_width, p_height);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            glfwTerminate();
            throw std::runtime_error("GLEW initialization failed");
        }
        glEnable(GL_DEPTH_TEST);
        glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
        //m_cube->initialize();
        InputHandler::init(m_window);
        renderer::init(p_width, p_height);
        m_manager.init();


        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // initialize audio
        audio.init();
        audio.playSound(soundID::bg, 0.1f);
    }

    ~GLApp() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    void update() {
        /* Poll for User Input */
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        InputHandler::update();
        
        // Press F9 to simulate a failure (for crash logger)
        if (glfwGetKey(m_window, GLFW_KEY_F9) == GLFW_PRESS) {
            throw std::runtime_error("F9 test: simulated failure in update()");
        }
        //const float deltaTime = 1.0f / 60.0f; // simulate 60fps
        const float deltaTime = static_cast<float>(m_delta);

        if (InputHandler::isKeyTriggered(GLFW_KEY_P)) {
            audio.toggleSoundPause(soundID::bg);
        }
        if (InputHandler::isMouseLeftClicked()) {
            audio.playSound(soundID::effect);
        }

        // input test code
        /*if (InputHandler::isKeyPressed(GLFW_KEY_SPACE))
            std::cout << InputHandler::getMouseY() << std::endl;*/
        m_inputSystem.update(m_manager, deltaTime);
        m_collisionSystem.update(m_manager, deltaTime);
        m_physicsSystem.update(m_manager, deltaTime);
        m_renderSystem.update(m_manager, deltaTime);
		m_uiSystem.update(m_manager, deltaTime);

        //m_angleOfRotation += m_rotationSpeed;
    }

    void render() {
        //glm::mat4 modelMat = glm::mat4(1.0f);
        //modelMat = glm::rotate(modelMat, m_angleOfRotation, glm::vec3(0.0f, 0.0f, 1.0f));
        //m_cube->render(modelMat);
        //renderer::update();

        //draw circles from imgui
        /*for (const auto& c : circles) {
            renderer::object temp;
            temp.model_ref = renderer::models[1];
            temp.translate = glm::vec3(c.getCenter().x, c.getCenter().y, 0.02);
            temp.scaling = glm::vec3(c.getRadius()+1, c.getRadius()+1, 2.0f + 1);
            temp.draw();
        }*/

        /*ImGui::Begin("OpenGL funny");
        ImGui::Text("fml");
        ImGui::End();*/

        // Render ImGui
        //ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        const float deltaTime = 1.0f / 60.0f;


        glfwSwapBuffers(m_window);
    }

    void run() {
        while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(m_window)){
            Perf::UpdateTime(m_delta, m_fps, 0.5);
            update();
            render();
            Perf::UpdateWindowTitle(m_window, m_title, m_fps, m_showFpsInTitle);
            audio.update();

        }
    }

public:
    float m_angleOfRotation;
    float m_rotationSpeed;
    void SetShowFpsInTitle(bool v) { m_showFpsInTitle = v; };

private:
    GameObjectManager m_manager;
    InputSystem m_inputSystem;
    CollisionSystem m_collisionSystem;
    PhysicsSystem m_physicsSystem;
    RenderSystem m_renderSystem;
	UISystem m_uiSystem;

    GLFWwindow* m_window;
    AudioHandler& audio = AudioHandler::getInstance();
    //std::shared_ptr<Cube> m_cube;
    //renderer::model model;
    double        m_fps = 0.0;
    double        m_delta = 0.0;
    std::string   m_title;
    bool m_showFpsInTitle = true; // default; main() will override from config

};

int main() {
#if defined(DEBUG) | defined(_DEBUG)
    // Enable run-time memory check for debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
   
    CrashLog::Init("crash_log.txt");


    try {

        //throw std::runtime_error("testing");

        AppConfig cfg;
        std::string err;
        LoadConfig("../projects/my-gl-app/resources/config.txt", cfg, &err);
        if (!err.empty()) std::cout << err << "\n";

        GLApp app(cfg.width, cfg.height, cfg.title.c_str());
        app.SetShowFpsInTitle(cfg.show_fps_in_title);
        glfwSwapInterval(cfg.vsync ? 1 : 0);  // vsync on/off
        glClearColor(cfg.clear_color[0],
            cfg.clear_color[1],
            cfg.clear_color[2],
            cfg.clear_color[3]);
        app.run();
        CrashLog::Shutdown();
        return 0;
    }
    catch (const std::exception& e) {
        // 2) Log the exception message
        CrashLog::WriteException(e.what());
        CrashLog::Shutdown();
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        CrashLog::WriteException("Unknown exception");
        CrashLog::Shutdown();
        std::cerr << "Unhandled unknown exception\n";
        return -1;
    }
}

