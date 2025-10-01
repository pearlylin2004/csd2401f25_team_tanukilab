/* Start Header ************************************************************************/
/*!
\file		main.cpp
\author     Hugo Low Ren Hao, low.h, 2402272, 5%
\par        low.h@digipen.edu
\date		October, 1st, 2025
\brief      Main entry point for the OpenGL application.
\file		main.cpp
\author
\par
\date		October 1, 2025
\brief		Main entry point for the application.
*/
/* End Header **************************************************************************/

#include <iostream>
#include <stdexcept>
#include <memory>
#include "CoreEngine.h"
#include "debug.h"

// This allows for memory leak detection in debug builds
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif

int main() {
#if defined(DEBUG) | defined(_DEBUG)
    // Enable run-time memory check for debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize the crash logger
    CrashLog::Init("crash_log.txt");

    // Create the engine using a smart pointer for automatic cleanup
    auto engine = std::make_unique<CoreEngine>();

    try {
        engine->Init();
        engine->Run();
    }
    catch (const std::exception& e) {
        CrashLog::WriteException(e.what());
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        engine->Shutdown();
        return -1;
    }
    catch (...) {
        CrashLog::WriteException("Unknown exception");
        std::cerr << "Unhandled unknown exception\n";
        engine->Shutdown();
        return -1;
    }

    // Gracefully shut down the engine and logger
    engine->Shutdown();
    CrashLog::Shutdown();
    return 0;
}