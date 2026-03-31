/* Start Header ************************************************************************/
/*!
\file		main.cpp
\author     Hugo Low Ren Hao, low.h, 2402272, 40%
\author     Varick, v.teo, 2403417, 40%
            Tan Chee Qing, 2401486, 10%
\par        low.h@digipen.edu
\par        v.teo@digipen.edu
            cheeqing.tan@digipen.edu
\date		March, 11st, 2026
\brief      Main entry point for the Waterbound.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "CoreEngine.h"
#include "LuaGameScript.h"

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

        // load menu as fir
        SceneManager::getInstance().executeLevelLoad("menu_scene.json");
        // register gamescript
        //engine->setGameScript(gameApp.get());

        luaScript = std::make_unique<LuaGameScript>(engine->getLuaSystem());
        engine->setGameScript(luaScript.get());

        // only work for game build
        engine->setOnUpdate([&]() {
            // F1 or SHARE on gamepad to quit
            if (InputHandler::isKeyTriggered(GLFW_KEY_F1) || InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_BACK))
                engine->stop();

            GameState current = GUISystem::getInstance().getCurrentState();

            // ESC (keyboard) -> pause
            if (InputHandler::isKeyTriggered(GLFW_KEY_ESCAPE)) {
                if (current != GameState::PAUSED && current != GameState::MENU) {
                    GUISystem::getInstance().setStateBeforePause(current);
                    GUISystem::getInstance().changeState(GameState::PAUSED, "pause_scene.json");
                }
            }

            // START (gamepad) -> main menu
            if (InputHandler::isGamepadButtonTriggered(GLFW_GAMEPAD_BUTTON_START)) {
                if (current != GameState::MENU) {
                    GUISystem::getInstance().changeState(GameState::MENU, "menu_scene.json");
                }
            }
            });

        GlobalRegistry::Instance().setBasePath("./assets");
        GlobalRegistry::Instance().init();

        // load menu as fir
        GUISystem::getInstance().loadScreen(engine->getManager(), "menu_scene.json");

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