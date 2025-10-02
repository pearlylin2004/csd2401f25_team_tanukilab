/* Start Header ************************************************************************/
/*!
\file		debug.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Crash logging and lightweight performance utilities.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "debug.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <mutex>
#include <ctime>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// ======================================================================================
// Internal helpers (anonymous namespace): thread-safe file append + time formatting
// ======================================================================================
namespace {
    // Shared singletons hidden in anonymous namespace
    std::mutex& LogMutex() { static std::mutex m; return m; }
    std::string& LogPath() { static std::string p = "crash_log.txt"; return p; }
    bool& Inited() { static bool inited = false; return inited; }

    /**
     * @brief Produce a local-time timestamp "YYYY-MM-DD HH:MM:SS".
     */
    std::string Timestamp()
    {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    /**
     * @brief Append a single line to the log file with a timestamp.
     *        Thread-safe via a mutex.
     * @param line Text to append (without trailing newline).
     */
    void AppendLine(const std::string& line)
    {
        std::lock_guard<std::mutex> lock(LogMutex());
        std::ofstream out(LogPath(), std::ios::app);
        if (!out) return; // if logging fails, avoid throwing in a crash path
        out << "[" << Timestamp() << "] " << line << "\n";
    }

    // --------- signal/exception/terminate handlers (log then abort) ----------

    /**
     * @brief POSIX signal handler. Logs signal type then aborts.
     */
    void SigHandler(int sig)
    {
        switch (sig) {
        case SIGSEGV: AppendLine("Crash: SIGSEGV (segmentation fault)"); break;
        case SIGABRT: AppendLine("Crash: SIGABRT (abort)"); break;
        default:      AppendLine("Crash: signal " + std::to_string(sig)); break;
        }
        std::abort(); // terminate immediately after logging
    }

#ifdef _WIN32
    /**
     * @brief Windows SEH filter for unhandled exceptions (e.g., access violation).
     *        Logs a generic SEH message and returns control to the OS.
     */
    LONG WINAPI SehFilter(EXCEPTION_POINTERS* info)
    {
        (void)info;
        AppendLine("Crash: Unhandled SEH (Windows structured exception)");
        return EXCEPTION_EXECUTE_HANDLER; // hand back to OS after logging
    }
#endif

    /**
     * @brief C++ terminate handler. Logs and aborts (likely due to unhandled exception).
     */
    void TerminateHandler()
    {
        AppendLine("Crash: std::terminate called (likely unhandled exception)");
        std::abort();
    }
} // anonymous namespace

// ======================================================================================
// CrashLog API
// ======================================================================================
namespace CrashLog {

    /**
     * @brief Initialize crash logging and register failure handlers.
     * @param logPath Optional path to the log file; defaults to "crash_log.txt".
     *
     * Safe to call multiple times; subsequent calls are ignored.
     */
    void Init(const char* logPath)
    {
        if (Inited()) return;
        if (logPath && *logPath) LogPath() = logPath;

        // Mark the start of the process in the log
        AppendLine("===== Application start =====");

        // Hook C++ terminate
        std::set_terminate(::TerminateHandler);

        // Register POSIX signals
        std::signal(SIGSEGV, ::SigHandler);
        std::signal(SIGABRT, ::SigHandler);

#ifdef _WIN32
        // Windows SEH for access violations and similar
        SetUnhandledExceptionFilter(::SehFilter);
#endif

        Inited() = true;
    }

    /**
     * @brief Write an informational line (prefixed with "Note: ") to the log.
     */
    void WriteLine(const char* msg)
    {
        if (!msg) return;
        AppendLine(std::string("Note: ") + msg);
    }

    /**
     * @brief Write details of an unhandled C++ exception message to the log.
     * @param what Exception what() string (may be null).
     */
    void WriteException(const char* what)
    {
        std::string line = "Unhandled C++ exception: ";
        line += (what ? what : "(null)");
        AppendLine(line);
    }

    /**
     * @brief Mark the end of the application in the log.
     */
    void Shutdown()
    {
        AppendLine("===== Application end =====");
    }
} // namespace CrashLog


// ======================================================================================
// Perf utilities: delta-time, rolling FPS, and window title updates
// ======================================================================================
namespace Perf {

    /**
     * @brief Update frame delta time and compute rolling FPS over an interval.
     *
     * Internally tracks previous time, a frame counter, and a start time for
     * the current FPS window. When the window exceeds @p fps_calc_interval,
     * the FPS value is refreshed as (frames / elapsed).
     *
     * @param[out] delta  Seconds elapsed since last call.
     * @param[out] fps    Frames per second (updated at the chosen interval).
     * @param[in]  fps_calc_interval  Seconds between FPS recomputations.
     *                                Clamped to [0, 10]. Use 0 for per-frame updates.
     */
    void UpdateTime(double& delta, double& fps, double fps_calc_interval) {
        // Track previous frame time
        static double prev_time = glfwGetTime();
        const  double curr_time = glfwGetTime();
        delta = curr_time - prev_time;
        prev_time = curr_time;

        // Rolling FPS accumulators
        static double count = 0.0;
        static double start_time = glfwGetTime();
        const  double elapsed_time = curr_time - start_time;

        ++count;

        // Keep the interval within a reasonable range
        if (fps_calc_interval < 0.0) fps_calc_interval = 0.0;
        if (fps_calc_interval > 10.0) fps_calc_interval = 10.0;

        // Recompute FPS when the interval has passed
        if (elapsed_time > fps_calc_interval) {
            fps = count / elapsed_time;
            start_time = curr_time;
            count = 0.0;
        }
    }

    /**
     * @brief Update the OS window title with a base title and optional FPS figure.
     * @param window  GLFW window handle.
     * @param title   Base text to display.
     * @param fps     Frames per second (from UpdateTime).
     * @param showFps If true, appends " | FPS: xx.x" to the title.
     */
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
