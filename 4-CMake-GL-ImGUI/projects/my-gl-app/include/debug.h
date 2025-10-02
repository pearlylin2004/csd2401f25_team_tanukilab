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

// --- Crash logging -----------------------------------------------------------
namespace CrashLog {
    // Call once at program start. 'logPath' can be relative (next to the exe)
    // e.g. "crash_log.txt" or "logs/crash_log.txt".
    void Init(const char* logPath = "crash_log.txt");

    // Optional: write a custom line to the log (e.g., before shutdown).
    void WriteLine(const char* msg);

    // If you already have a try/catch in main, call this in the catch block.
    void WriteException(const char* what);

    // Optional: flush/close. (Safe to omit; file is closed automatically.)
    void Shutdown();
}
