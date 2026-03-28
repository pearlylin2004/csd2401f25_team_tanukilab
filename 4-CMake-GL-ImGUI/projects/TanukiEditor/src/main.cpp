
#include "CoreEngine.h"
#include "LuaGameScript.h"
#include "editorLaunch.h"
#include "PathManager.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

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
            forceWindowed = true;
    }

    // Create the engine using a smart pointer for automatic cleanup
    auto engine = std::make_unique<CoreEngine>();
    std::unique_ptr<LuaGameScript> luaScript;

    try {
        engine->Init(forceWindowed);

        // enable editor by default
        UISystem::toggleUI();
        EditorManager::toggleEditing(true); 
        EditorManager::setCanToggleEditor(false); // cannot ctrl-0 to hide editor

        // run launcher
        //engine->runLauncher([&]() {return EditorLaunch::Instance().render();});
        engine->setLauncher([&]() { 
            bool done = EditorLaunch::Instance().render();
            if (done) {
                PathManager::Instance().setSourcePath(EditorLaunch::Instance().getGamePath());

                Editor::assetBrowserState.currentFolder = PathManager::Instance().getSourcePath() + "/assets/";
                Editor::scriptState.scriptDir = PathManager::Instance().getSourceScriptPath();

                std::string defaultScene = EditorLaunch::Instance().getDefaultScene();
                Editor::sceneState.currentSceneName = defaultScene;

                PrefabManager::Instance().loadPrefabRegistry();
                GlobalRegistry::Instance().setBasePath(PathManager::Instance().getSourcePath() + "/assets");
                GlobalRegistry::Instance().init();

                if (!defaultScene.empty())
                    engine->loadInitialScene(defaultScene);
            }
            return done;
        });

        // set project filepath
        //PathManager::Instance().setSourcePath(
           // EditorLaunch::Instance().getGamePath());

        luaScript = std::make_unique<LuaGameScript>(engine->getLuaSystem());
        engine->setGameScript(luaScript.get());

        // open first scene in scene folder
        /*std::string defaultScene = EditorLaunch::Instance().getDefaultScene();
        if (!defaultScene.empty())
            engine->loadInitialScene(defaultScene);*/

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