/* Start Header ************************************************************************/
/*!
\file		Engine.h
\author
\par
\date		Month, Day, 2025
\brief
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Engine {
public:
    // lifecycle
    bool init(int width, int height, const std::string& title);
    void run();
    void shutdown();

    // timing / perf
    void  update_time(double fps_calc_interval = 1.0);
    inline double fps()   const { return m_fps; }
    inline double delta() const { return m_delta; }

    // accessors
    inline GLFWwindow* window() const { return m_window; }
    inline int  width()  const { return m_width; }
    inline int  height() const { return m_height; }
    inline const std::string& title() const { return m_title; }

private:
    
    GLFWwindow* m_window = nullptr;
    int           m_width = 0;
    int           m_height = 0;
    std::string   m_title;
    double        m_fps = 0.0;
    double        m_delta = 0.0;

    
    double m_msInput = 0.0, m_msRender = 0.0, m_msPresent = 0.0;

    // ==== setup helpers ====
    void setup_event_callbacks();
    void update_title_bar();

   
    static void error_cb(int error, const char* description);
    static void fbsize_cb(GLFWwindow* win, int width, int height);
    static void key_cb(GLFWwindow* win, int key, int scancode, int action, int mods);
    static void mousebutton_cb(GLFWwindow* win, int button, int action, int mods);
    static void mousepos_cb(GLFWwindow* win, double xpos, double ypos);
    static void mousescroll_cb(GLFWwindow* win, double xoffset, double yoffset);

    // previous callbacks (for chaining with InputHandler)
    static GLFWkeyfun            s_prevKeyCb;
    static GLFWmousebuttonfun    s_prevMouseBtnCb;
    static GLFWcursorposfun      s_prevCursorPosCb;
    static GLFWscrollfun         s_prevScrollCb;
    static GLFWframebuffersizefun s_prevFbSizeCb;

};
