/* Start Header ************************************************************************/
/*!
\file		debug.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Declarations for crash logging and performance utilities.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// ======================================================================================
// Perf namespace: frame timing & FPS display
// ======================================================================================
namespace Perf {

    /**
     * @brief Update frame delta time and compute rolling FPS.
     *
     * Call once per frame. Internally maintains state across calls.
     *
     * @param[out] delta   Seconds between frames.
     * @param[out] fps     Frames per second, smoothed over `fps_calc_interval`.
     * @param[in]  fps_calc_interval Interval (seconds) over which FPS is averaged.
     *                               Default = 0.5. Clamped to [0, 10].
     */
    void UpdateTime(double& delta, double& fps, double fps_calc_interval = 0.5);

    /**
     * @brief Update the GLFW window title with a base title and optional FPS.
     *
     * @param window   GLFW window handle.
     * @param title    Base title string.
     * @param fps      Current frames per second (calculated by UpdateTime).
     * @param showFps  If true, appends " | FPS: xx.x" to the title.
     */
    void UpdateWindowTitle(GLFWwindow* window,
        const std::string& title,
        double fps,
        bool showFps);

} // namespace Perf

// ======================================================================================
// CrashLog namespace: crash logging & signal handling
// ======================================================================================
namespace CrashLog {

    /**
     * @brief Initialize crash logging and install handlers.
     *
     * Safe to call once at program start. Creates/uses a log file (default "crash_log.txt")
     * and writes a start marker. Registers terminate handler, signal handlers,
     * and (on Windows) SEH filter.
     *
     * @param logPath Optional file path for log output.
     */
    void Init(const char* logPath = "crash_log.txt");

    /**
     * @brief Write a custom informational line to the crash log.
     *
     * @param msg Message string to log. Ignored if null.
     */
    void WriteLine(const char* msg);

    /**
     * @brief Log an unhandled C++ exception message.
     *
     * Typically called inside a catch block if you want to record exception details.
     *
     * @param what The `what()` string from std::exception, or custom message.
     */
    void WriteException(const char* what);

    /**
     * @brief Mark the end of the application in the log file.
     *
     * Safe to omit; the file will close automatically when the process exits.
     */
    void Shutdown();
}