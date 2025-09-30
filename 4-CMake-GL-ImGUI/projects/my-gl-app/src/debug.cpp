// debug_fps.cpp
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

namespace {
    std::mutex& LogMutex() { static std::mutex m; return m; }
    std::string& LogPath() { static std::string p = "crash_log.txt"; return p; }
    bool& Inited() { static bool inited = false; return inited; }

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

    void AppendLine(const std::string& line)
    {
        std::lock_guard<std::mutex> lock(LogMutex());
        std::ofstream out(LogPath(), std::ios::app);
        if (!out) return;
        out << "[" << Timestamp() << "] " << line << "\n";
    }

    // --------- signal handlers ----------
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
    LONG WINAPI SehFilter(EXCEPTION_POINTERS* info)
    {
        (void)info;
        AppendLine("Crash: Unhandled SEH (Windows structured exception)");
        return EXCEPTION_EXECUTE_HANDLER; // hand back to OS after logging
    }
#endif

    // --------- terminate handler ----------
    void TerminateHandler()
    {
        AppendLine("Crash: std::terminate called (likely unhandled exception)");
        std::abort();
    }
}

// ----------------- API --------------------
namespace CrashLog {

    void Init(const char* logPath)
    {
        if (Inited()) return;
        if (logPath && *logPath) LogPath() = logPath;

        // mark start
        AppendLine("===== Application start =====");

        // C++ terminate hook
        std::set_terminate(::TerminateHandler);

        // POSIX-style signals
        std::signal(SIGSEGV, ::SigHandler);
        std::signal(SIGABRT, ::SigHandler);

#ifdef _WIN32
        // Windows SEH (access violation, etc.)
        SetUnhandledExceptionFilter(::SehFilter);
#endif

        Inited() = true;
    }

    void WriteLine(const char* msg)
    {
        if (!msg) return;
        AppendLine(std::string("Note: ") + msg);
    }

    void WriteException(const char* what)
    {
        std::string line = "Unhandled C++ exception: ";
        line += (what ? what : "(null)");
        AppendLine(line);
    }

    void Shutdown()
    {
        AppendLine("===== Application end =====");
    }
}


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
