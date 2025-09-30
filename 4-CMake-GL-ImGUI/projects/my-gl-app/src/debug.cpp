// debug_fps.cpp
#include "debug.h"
#include <sstream>
#include <iomanip>

namespace Perf {

    void UpdateTime(double& delta, double& fps, double fps_calc_interval) {
        static double prev_time = glfwGetTime();
        const  double curr_time = glfwGetTime();
        delta = curr_time - prev_time;
        prev_time = curr_time;

        static double count = 0.0;
        static double start_time = glfwGetTime();
        const  double elapsed_time = curr_time - start_time;

        ++count;

        if (fps_calc_interval < 0.0) fps_calc_interval = 0.0;
        if (fps_calc_interval > 10.0) fps_calc_interval = 10.0;

        if (elapsed_time > fps_calc_interval) {
            fps = count / elapsed_time;
            start_time = curr_time;
            count = 0.0;
        }
    }

    void UpdateWindowTitle(GLFWwindow* window,
        const std::string& title,
        double fps,
        bool showFps) {
        if (!window) return;

        std::ostringstream oss;
        oss << title;
        if (showFps) {
            oss << " | FPS: " << std::fixed << std::setprecision(1) << fps;
        }
        glfwSetWindowTitle(window, oss.str().c_str());
    }

} // namespace Perf
