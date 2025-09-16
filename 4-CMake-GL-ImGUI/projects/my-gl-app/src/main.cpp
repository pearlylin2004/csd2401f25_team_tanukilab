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
//class Cube {
//public:
//    Cube() : m_programId(0), m_vertexBuffer(0), m_vertexArrayId(0) {}
//    ~Cube() {
//        glDeleteBuffers(1, &m_vertexBuffer);
//        glDeleteVertexArrays(1, &m_vertexArrayId);
//        glDeleteProgram(m_programId);
//    }
//
//    void initialize() {
//
//        m_programId = LoadShaders("shaders/funny.vert", "shaders/funny.frag", true);
//        m_geometryBuffer = {
//            //  x      y     z      r     g     b
//            -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 1.0f,   // Bottom-left, cyan
//             0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 1.0f,   // Bottom-right, magenta
//             0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   // Top-right, yellow
//
//             0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // Top-right (repeat), red
//            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   // Top-left, green
//            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f    // Bottom-left (repeat), blue
//        };
//        glGenVertexArrays(1, &m_vertexArrayId);
//        glBindVertexArray(m_vertexArrayId);
//        glGenBuffers(1, &m_vertexBuffer);
//        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
//        glBufferData(GL_ARRAY_BUFFER, m_geometryBuffer.size() * sizeof(GLfloat), m_geometryBuffer.data(), GL_STATIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
//    }
//
//    void render(const glm::mat4& p_modelMat) {
//        glUseProgram(m_programId);
//        glBindVertexArray(m_vertexArrayId);
//        GLint vTransformLoc = glGetUniformLocation(m_programId, "vertexTransform");
//        glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &p_modelMat[0][0]);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//    }
//
//private:
//    GLuint m_programId, m_vertexBuffer, m_vertexArrayId;
//    std::vector<GLfloat> m_geometryBuffer;
//};

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
        renderer::init();

        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    ~GLApp() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    void update() {
        /* Poll for User Input */
        InputHandler::update();

        // input test code
        /*if (InputHandler::isKeyPressed(GLFW_KEY_SPACE))
            std::cout << InputHandler::getMouseY() << std::endl;*/

        m_angleOfRotation += m_rotationSpeed;
    }

    void render() {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::mat4 modelMat = glm::mat4(1.0f);
        //modelMat = glm::rotate(modelMat, m_angleOfRotation, glm::vec3(0.0f, 0.0f, 1.0f));
        //m_cube->render(modelMat);
        renderer::draw();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create ImGui window
        ImGui::Begin("OpenGL 3.3 Demo");
        ImGui::Text("Hello, ImGui!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Rotation Speed", &m_rotationSpeed, 0.0f, 0.1f);
        //ImGui::SliderFloat("Rotation fk u", &m_rotationSpeed, 0.0f, 0.1f);
        if (ImGui::Button("Reset Rotation")) {
            m_angleOfRotation = 0.0f;
        }
        ImGui::End();

        /*ImGui::Begin("OpenGL funny");
        ImGui::Text("fml");
        ImGui::End();*/

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    void run() {
        while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(m_window)) {
            //renderer::update(m_window);
            render();
            //update();
            //renderer::draw();
            glfwPollEvents();
        }
    }

public:
    float m_angleOfRotation;
    float m_rotationSpeed;

private:
    GLFWwindow* m_window;
    //std::shared_ptr<Cube> m_cube;
    //renderer::model model;
};

int main() {
    try {
        GLApp app(800, 800, "Render Cube");
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}