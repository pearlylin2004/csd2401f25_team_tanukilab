#include "openGLwindow.h"

bool GLHelper::init(GLint width, GLint height, std::string title) {
    GLHelper::width = width;
    GLHelper::height = height;
    GLHelper::title = title;

    // Part 1
    if (!glfwInit()) {
        std::cout << "GLFW init has failed - abort program!!!" << std::endl;
        return false;
    }

    // In case a GLFW function fails, an error is reported to callback function
    glfwSetErrorCallback(GLHelper::error_cb);

    // Before asking GLFW to create an OpenGL context, we specify the minimum constraints
    // in that context:
    // Required OpenGL version
    constexpr int REQUIRED_GL_MAJOR = 4;
    constexpr int REQUIRED_GL_MINOR = 5;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, REQUIRED_GL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, REQUIRED_GL_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, 8); glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8); glfwWindowHint(GLFW_ALPHA_BITS, 8);

    GLHelper::ptr_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!GLHelper::ptr_window) {
        std::cerr << "GLFW unable to create OpenGL context - abort program\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(GLHelper::ptr_window);

    // this is the default setting ...
    glfwSetInputMode(GLHelper::ptr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Part 2: Initialize entry points to OpenGL functions and extensions
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Unable to initialize GLEW - error: "
            << glewGetErrorString(err) << " abort program" << std::endl;
        return false;
    }
    if (GLEW_VERSION_4_5) {
        std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
        std::cout << "Driver supports OpenGL 4.5\n" << std::endl;
    }
    else {
        const GLubyte* glVersionStr = glGetString(GL_VERSION);
        if (glVersionStr) {
            int major = 0, minor = 0;
            if (sscanf(reinterpret_cast<const char*>(glVersionStr), "%d.%d", &major, &minor) == 2) {
                if (major > REQUIRED_GL_MAJOR || (major == REQUIRED_GL_MAJOR && minor >= REQUIRED_GL_MINOR)) {
                    std::cout << "Driver supports OpenGL " << major << "." << minor << "\n" << std::endl;
                }
                else {
                    std::cerr << "Warning: Detected OpenGL version " << major << "." << minor
                        << " — does not meet required version "
                        << REQUIRED_GL_MAJOR << "." << REQUIRED_GL_MINOR << "." << std::endl;
                }
            }
            else {
                std::cerr << "Warning: Unable to parse OpenGL version string." << std::endl;
            }
        }
        else {
            std::cerr << "Warning: Unable to retrieve OpenGL version string." << std::endl;
        }
    }

    return true;
}


void GLHelper::cleanup() {
    // Part 1
    glfwTerminate();
}

void GLHelper::error_cb(int error, char const* description) {
#ifdef _DEBUG
    std::cerr << "GLFW error: " << description << std::endl;
#endif
}