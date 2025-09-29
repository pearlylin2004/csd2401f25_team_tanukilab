// engine.cpp
#include "engine.h"
#include "renderer.h"
#include "input.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

// ----------- previous-callback pointers for chaining -----------
GLFWkeyfun             Engine::s_prevKeyCb = nullptr;
GLFWmousebuttonfun     Engine::s_prevMouseBtnCb = nullptr;
GLFWcursorposfun       Engine::s_prevCursorPosCb = nullptr;
GLFWscrollfun          Engine::s_prevScrollCb = nullptr;
GLFWframebuffersizefun Engine::s_prevFbSizeCb = nullptr;

// ================== init / shutdown ==================
bool Engine::init(int w, int h, const std::string& ttl) {
    m_width = w; m_height = h; m_title = ttl;

    if (!glfwInit()) {
        std::cerr << "GLFW init has failed - abort program!!!\n";
        return false;
    }

    // Context constraints
    constexpr int REQUIRED_GL_MAJOR = 4;
    constexpr int REQUIRED_GL_MINOR = 5;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, REQUIRED_GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, REQUIRED_GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "GLFW unable to create OpenGL context - abort program\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);

    // GLEW init
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Unable to initialize GLEW - error: "
            << glewGetErrorString(err) << " abort program\n";
        return false;
    }

    if (GLEW_VERSION_4_5) {
        std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << "\n";
        std::cout << "Driver supports OpenGL 4.5\n\n";
    }
    else {
        const GLubyte* glVersionStr = glGetString(GL_VERSION);
        if (glVersionStr) {
            int major = 0, minor = 0;
            if (sscanf(reinterpret_cast<const char*>(glVersionStr), "%d.%d", &major, &minor) == 2) {
                if (major > REQUIRED_GL_MAJOR || (major == REQUIRED_GL_MAJOR && minor >= REQUIRED_GL_MINOR))
                    std::cout << "Driver supports OpenGL " << major << "." << minor << "\n\n";
                else
                    std::cerr << "Warning: Detected OpenGL " << major << "." << minor
                    << " < required " << REQUIRED_GL_MAJOR << "." << REQUIRED_GL_MINOR << "\n";
            }
            else std::cerr << "Warning: Unable to parse OpenGL version string.\n";
        }
        else   std::cerr << "Warning: Unable to retrieve OpenGL version string.\n";
    }

    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); // vsync

    // Initialize your systems first (these set their own callbacks)
    InputHandler::init(m_window);
    renderer::init(m_width, m_height);

    // After other systems have installed their callbacks, install ours and chain to them.
    setup_event_callbacks();

    // Cursor mode like the original helper
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    return true;
}

void Engine::shutdown() {
    // renderer::shutdown(); // if you add it later
    if (m_window) { glfwDestroyWindow(m_window); m_window = nullptr; }
    glfwTerminate();
}

// ================== main loop ==================
void Engine::run() {
    using clock = std::chrono::high_resolution_clock;
    while (!glfwWindowShouldClose(m_window)) {
        auto t0 = clock::now();

        glfwPollEvents();
        InputHandler::update();

        auto t1 = clock::now();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //renderer::update(); // your renderer draws here

        auto t2 = clock::now();

        glfwSwapBuffers(m_window);

        auto t3 = clock::now();

        // timings (for % breakdown)
        m_msInput = std::chrono::duration<double, std::milli>(t1 - t0).count();
        m_msRender = std::chrono::duration<double, std::milli>(t2 - t1).count();
        m_msPresent = std::chrono::duration<double, std::milli>(t3 - t2).count();

        // fps / delta
        update_time(1.0);
        update_title_bar();
    }
}

// ================== time  ==================
void Engine::update_time(double fps_calc_interval) {
    static double prev_time = glfwGetTime();
    const double curr_time = glfwGetTime();
    m_delta = curr_time - prev_time;
    prev_time = curr_time;

    static double count = 0.0;
    static double start_time = glfwGetTime();
    const double elapsed_time = curr_time - start_time;

    ++count;

    if (fps_calc_interval < 0.0) fps_calc_interval = 0.0;
    if (fps_calc_interval > 10.0) fps_calc_interval = 10.0;

    if (elapsed_time > fps_calc_interval) {
        m_fps = count / elapsed_time;
        start_time = curr_time;
        count = 0.0;
    }
}

// ================== callback setup (chain to previous) ==================
void Engine::setup_event_callbacks() {
    glfwSetErrorCallback(Engine::error_cb);

    // Capture previously installed callbacks (e.g., from InputHandler) and then set ours.
    s_prevFbSizeCb = glfwSetFramebufferSizeCallback(m_window, Engine::fbsize_cb);
    s_prevKeyCb = glfwSetKeyCallback(m_window, Engine::key_cb);
    s_prevMouseBtnCb = glfwSetMouseButtonCallback(m_window, Engine::mousebutton_cb);
    s_prevCursorPosCb = glfwSetCursorPosCallback(m_window, Engine::mousepos_cb);
    s_prevScrollCb = glfwSetScrollCallback(m_window, Engine::mousescroll_cb);
}

// ================== callbacks (print, then forward) ==================
void Engine::key_cb(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
#ifdef _DEBUG
        std::cout << "Key pressed\n";
#endif
    }
    else if (action == GLFW_REPEAT) {
#ifdef _DEBUG
        std::cout << "Key repeatedly pressed\n";
#endif
    }
    else if (action == GLFW_RELEASE) {
#ifdef _DEBUG
        std::cout << "Key released\n";
#endif
    }
    if (s_prevKeyCb) s_prevKeyCb(win, key, scancode, action, mods);
}

void Engine::mousebutton_cb(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
#ifdef _DEBUG
        std::cout << (button == GLFW_MOUSE_BUTTON_LEFT ? "Left" : "Right")
            << " mouse button " << (action == GLFW_PRESS ? "pressed!!!\n" : "released!!!\n");
#endif
    }
    if (s_prevMouseBtnCb) s_prevMouseBtnCb(win, button, action, mods);
}

void Engine::mousepos_cb(GLFWwindow* win, double xpos, double ypos) {
#ifdef _DEBUG
    std::cout << "Mouse cursor position: (" << (int)xpos << ", " << (int)ypos << ")\n";
#endif
    if (s_prevCursorPosCb) s_prevCursorPosCb(win, xpos, ypos);
}

void Engine::mousescroll_cb(GLFWwindow* win, double xoffset, double yoffset) {
#ifdef _DEBUG
    std::cout << "Mouse scroll wheel offset: (" << xoffset << ", " << yoffset << ")\n";
#endif
    if (s_prevScrollCb) s_prevScrollCb(win, xoffset, yoffset);
}

void Engine::error_cb(int /*error*/, const char* description) {
#ifdef _DEBUG
    std::cerr << "GLFW error: " << description << "\n";
#endif
}

void Engine::fbsize_cb(GLFWwindow* win, int w, int h) {
#ifdef _DEBUG
    std::cout << "fbsize_cb getting called!!!\n";
#endif
    if (s_prevFbSizeCb) s_prevFbSizeCb(win, w, h);
}

// ================== title bar (FPS + %) ==================
void Engine::update_title_bar() {
    const double frame = m_msInput + m_msRender + m_msPresent;
    auto pct = [&](double ms) { return frame > 0.0 ? (ms / frame * 100.0) : 0.0; };

    std::ostringstream oss;
    oss << m_title
        << " | FPS: " << std::fixed << std::setprecision(1) << m_fps
        << " | " << std::setprecision(2) << frame << " ms"
        << " | Input " << std::setprecision(1) << pct(m_msInput) << '%'
        << " | Render " << std::setprecision(1) << pct(m_msRender) << '%'
        << " | Present " << std::setprecision(1) << pct(m_msPresent) << '%';

    glfwSetWindowTitle(m_window, oss.str().c_str());
}
