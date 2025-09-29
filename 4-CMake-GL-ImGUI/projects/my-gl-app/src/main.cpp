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
#include "collision.h"
#include "engine.h"
#include "ui.h"
#include "GameObjectManager.h"
#include "Component.h"
#include "Systems.h"


//collision testing
//helper function to get an AABB from a renderer object
Collision::AABB getObjectAABB(const renderer::object& obj) {
    Vector2D center(obj.translate.x + obj.scaling.x * 0.5f,
        obj.translate.y + obj.scaling.y * 0.5f);

    float halfWidth = obj.scaling.x * 0.5f;
    float halfHeight = obj.scaling.y * 0.5f;

    Vector2D min(center.x - halfWidth, center.y - halfHeight);
    Vector2D max(center.x + halfWidth, center.y + halfHeight);
    return Collision::AABB(min, max);
}

Collision::Circle getObjectCircle(const renderer::object& obj) {
    Vector2D center(obj.translate.x + obj.scaling.x * 0.5f,
        obj.translate.y + obj.scaling.y * 0.5f);
    float radius = obj.scaling.x * 0.5f;
    return Collision::Circle(center, radius);
}
//std::string outputC = "No collision";
//to test adding of circles for imgui
std::vector<Collision::Circle> circles;
//static std::vector<std::string> debugMessages;


class GLApp {
public:
    GLApp(int p_width, int p_height, const char* p_title) : m_angleOfRotation(0.0f),
        m_rotationSpeed(0.01f),
        m_window(nullptr)//, 
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

        const float deltaTime = 1.0f / 60.0f; // simulate 60fps

        // input test code
        /*if (InputHandler::isKeyPressed(GLFW_KEY_SPACE))
            std::cout << InputHandler::getMouseY() << std::endl;*/
        m_inputSystem.update(m_manager, deltaTime);
        m_physicsSystem.update(m_manager, deltaTime);
        m_renderSystem.update(m_manager, deltaTime);
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

        //how many circles
        //std::cout << circles.size() << std::endl;
             
        //start ImGui frame -- just need to call once
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //imgui frames
        UI::drawDemo(m_rotationSpeed, m_angleOfRotation);
        UI::drawControls(circles);
		//UI::drawDebug(outputC);
        UI::drawDebug(UI::debugMessages);

        /*ImGui::Begin("OpenGL funny");
        ImGui::Text("fml");
        ImGui::End();*/

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        const float deltaTime = 1.0f / 60.0f;
        

        glfwSwapBuffers(m_window);
    }

    void run() {
        /*renderer::objects["player"].translate = glm::vec3(0, 0, 0.0f);
        while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(m_window)) {
            Vector2D movement(0, 0);
            //renderer::update(m_window);
            if (InputHandler::isKeyHeld(GLFW_KEY_A))
                //renderer::objects.find("player")->second.movex(-0.01f);
                movement.x -= 0.01f;
            if (InputHandler::isKeyHeld(GLFW_KEY_D))
                //renderer::objects.find("player")->second.movex(0.01f);
                movement.x += 0.01f;    
            if (InputHandler::isKeyHeld(GLFW_KEY_W))
                //renderer::objects.find("player")->second.movey(0.01f);
                movement.y += 0.01f;
            if (InputHandler::isKeyHeld(GLFW_KEY_S))
                //renderer::objects.find("player")->second.movey(-0.01f);
                movement.y -= 0.01f;

            render();
            //update();
            //renderer::draw();
            //glfwPollEvents();
            InputHandler::update();
        }*/
        renderer::objects["player"].translate = glm::vec3(0, 0, 0.0f);

        while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            !glfwWindowShouldClose(m_window)) {

            Vector2D movement{ 0, 0 };
            if (InputHandler::isKeyHeld(GLFW_KEY_A)) movement.x -= 0.01f;
            if (InputHandler::isKeyHeld(GLFW_KEY_D)) movement.x += 0.01f;
            if (InputHandler::isKeyHeld(GLFW_KEY_W)) movement.y += 0.01f;
            if (InputHandler::isKeyHeld(GLFW_KEY_S)) movement.y -= 0.01f;

            Collision::AABB playerAABB = getObjectAABB(renderer::objects["player"]);
            Collision::Circle platformCircle = getObjectCircle(renderer::objects["platform"]);

            Vector2D circleVel{ 0, 0 };
            Vector2D playerVel = movement;

            //dynamic collision
            Collision::CollisionInfo info = Collision::CollisionIntersection_CircleAABB_Dynamic_Info(
                platformCircle, circleVel, playerAABB, playerVel
            );

            if (info.collided) {
                UI::debugMessages.push_back(
                    "Collision with platform! t=" + std::to_string(info.timeOfImpact)
                );

                //stop player at contact
                Collision::Resolve_Circle_AABB(
                    platformCircle, circleVel, 0.0f,  
                    playerAABB, playerVel, 1.0f, 
                    info, Collision::CollisionResponseMode::Stop
                );
            }

            renderer::objects["player"].movex(playerVel.x);
            renderer::objects["player"].movey(playerVel.y);

            render();
            InputHandler::update();
        
        while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(m_window)) 
        {
            update();
            render();
            glfwPollEvents();
        }
    }

public:
    float m_angleOfRotation;
    float m_rotationSpeed;

private:
    GameObjectManager m_manager;
	InputSystem m_inputSystem;
	PhysicsSystem m_physicsSystem;
	RenderSystem m_renderSystem;

    GLFWwindow* m_window;
    //std::shared_ptr<Cube> m_cube;
    //renderer::model model;
};

int main() {
    try {
        GLApp app(1600, 800, "Render Cube");
        app.run();
        //Engine engine;

        //if (!engine.init(1600, 800, "Tanuki Lab")) {
        //    return -1; // failed to initialize
        //}

        //engine.run();       // main loop (handles input, rendering, FPS, etc.)
        //engine.shutdown();  // cleanup resources

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return -1;
    }
}