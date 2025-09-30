// debug.h
#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Perf {

    // Updates delta time and rolling FPS. Call once per frame.
    // - delta   (out): seconds between frames
    // - fps     (out): frames per second, smoothed over 'fps_calc_interval'
    void UpdateTime(double& delta, double& fps, double fps_calc_interval = 0.5);

    // Updates the window title with "title | FPS: X.Y" if showFps is true.
    // (If false, it leaves the title as-is.)
    void UpdateWindowTitle(GLFWwindow* window,
        const std::string& title,
        double fps,
        bool showFps);

} // namespace Perf
