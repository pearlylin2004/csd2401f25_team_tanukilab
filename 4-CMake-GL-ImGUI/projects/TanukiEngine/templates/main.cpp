// template main.cpp that auto clone to created game's src folder

#include "CoreEngine.h"
#include "LuaGameScript.h"
#include "GUISystem.h"
#include "globalRegistry.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

// This allows for memory leak detection in debug builds
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char** argv) {
#if defined(DEBUG) | defined(_DEBUG)
    // Enable run-time memory check for debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize the crash logger
    CrashLog::Init("crash_log.txt");

    bool forceWindowed = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--windowed")
        {
            forceWindowed = true;
        }
    }

    // Create the engine using a smart pointer for automatic cleanup
    auto engine = std::make_unique<CoreEngine>();
    std::unique_ptr<LuaGameScript> luaScript;


    try {
        engine->Init(forceWindowed);

        // set up lua game script
        luaScript = std::make_unique<LuaGameScript>(engine->getLuaSystem());
        engine->setGameScript(luaScript.get());

        // set up file global settings
        GlobalRegistry::Instance().setBasePath("./assets");
        GlobalRegistry::Instance().init();

        // load first scene
        GUISystem::getInstance().loadScreen(engine->getManager(), "new.json");

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