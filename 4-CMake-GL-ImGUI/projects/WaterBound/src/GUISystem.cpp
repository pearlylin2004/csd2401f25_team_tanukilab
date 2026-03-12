/* Start Header ************************************************************************/
/*!
\file       GUISystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 5% of the file
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 55% of the file
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 10% of the file
\par        asha.m@digipen.edu
\par        p.lin@digipen.edu
\par        cheeqing.tan@digipen.edu
\date       November, 7th, 2025
\brief      Handles initialization, updates, and GUI interactions such as
            creating/removing menu buttons and transitioning between
            MENU, CONTROl and PLAYING states.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include "CoreEngine.h"
#include "LeaderboardSystem.h"
#include "ReplaySystem.h"
#include <iostream>
#include <fstream>

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

/*GUISystem::GUISystem() :
    m_currentState(GameState::NONE),
    m_previousState(GameState::MENU),
    m_stateBeforePause(GameState::MENU),
    g_manager(nullptr),
    g_luaSystem(nullptr) {
    std::cout << "[GUI] GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "[GUI] GUISystem destructor called" << std::endl;
}*/

void GUISystem::init(GameObjectManager& manager, LuaSystem& luaSystem, VisualLogicSystem& vlSystem) {
    std::cout << "[GUI] GUISystem::init started" << std::endl;

    g_manager = &manager;
    g_luaSystem = &luaSystem;
    g_visualLogicSystem = &vlSystem;
    std::cout << "[GUI] GUISystem::init completed successfully" << std::endl;

    // Load the menu scene - this will populate button data in the manager
    loadScreen(manager, "menu_scene.json");

    
}

//update function to check for button clicking
void GUISystem::update() {
    // Update in-game HUD every frame during gameplay
    updateInGameHUD();

    if (m_pendingRestart) {
        // Ignore restarts during replay viewer
        if (m_currentState == GameState::REPLAY) {
            m_pendingRestart = false;
            return;
        }
        m_pendingRestart = false;
        restartLevel();
        return;
    }

    if (m_pendingLevelLoad && !UISystem::isShowUI()) {
        // Ignore visual-logic level transitions during replay viewer
        if (m_currentState == GameState::REPLAY) {
            m_pendingLevelLoad = false;
            m_pendingLevelName = "";
            return;
        }
        m_pendingLevelLoad = false;
        std::string levelToLoad = m_pendingLevelName;
        m_pendingLevelName = "";
        if (levelToLoad == "win_scene.json") {
            // Stop timers
            m_levelTimerRunning = false;
            m_gameTimerRunning = false;

            // Use per-level timer for the replay ghost save
            int levelTimeMs = static_cast<int>(m_levelTimer * 1000.0);
            ReplaySystem::getInstance().stopRecording(levelTimeMs);
            ReplaySystem::getInstance().stopPlayback(*g_manager);

            // Use total game timer for the leaderboard entry
            int totalTimeMs = static_cast<int>(m_gameTimer * 1000.0);
            LeaderboardSystem::getInstance().addEntry("Player", totalTimeMs);
            std::cout << "[GUI] Game completed in " << m_gameTimer << "s (" << totalTimeMs << "ms)\n";

            // Finalize the run: move current/ to <totalTimeMs>/
            ReplaySystem::getInstance().finalizeRun(totalTimeMs);
            m_lastRunId = std::to_string(totalTimeMs);

            loadScreen(*g_manager, levelToLoad);
        }
        else {
            // Save replay for the current level before moving to the next level
            int completionTimeMs = static_cast<int>(m_levelTimer * 1000.0);
            ReplaySystem::getInstance().stopRecording(completionTimeMs);
            ReplaySystem::getInstance().stopPlayback(*g_manager);

            loadLevel(levelToLoad);
        }
        return;
    }

    if (m_pendingAction != ButtonAction::NONE) {
        auto actionIt = m_buttonActions.find(m_pendingAction);
        m_pendingAction = ButtonAction::NONE;
        if (actionIt != m_buttonActions.end()) actionIt->second();
        return;
    }

    // Replay selection screen: handle navigation
    if (m_replaySelectActive && m_currentState == GameState::REPLAY) {
        handleReplaySelectInput();
        return;
    }

    // Replay viewer: feed recorded inputs through the game simulation
    if (m_replayViewerActive && m_currentState == GameState::REPLAY) {
        auto& replay = ReplaySystem::getInstance();

        // Handle playback controls (pause, seek, speed, frame step)
        handleReplayViewerControls();

        // Update replay HUD overlay
        updateReplayHUD();

        // When the viewer finishes the current level, advance to the next
        if (replay.isViewerFinished() && !replay.isViewerPaused()) {
            replay.stopViewer();
            m_replayLevelIndex++;
            if (m_replayLevelIndex < static_cast<int>(m_replayLevels.size())) {
                loadReplayLevel(m_replayLevelIndex);
            }
            else {
                // All levels replayed — return to menu
                m_replayViewerActive = false;
                changeState(GameState::MENU, "menu_scene.json");
            }
        }

        // ESC exits replay viewer back to menu
        if (InputHandler::isKeyTriggered(GLFW_KEY_ESCAPE)) {
            replay.stopViewer();
            m_replayViewerActive = false;
            changeState(GameState::MENU, "menu_scene.json");
        }
        return;
    }

    if (EditorManager::isEditingMode()) return;
    //if (m_currentState == GameState::PLAYING) return;
    if (UISystem::isShowUI()) return; // skip if UI is shown
    //if (m_buttons.empty()) return;

    Vector2D worldMousePos = InputHandler::getWorldMousePosition();

    //if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
    //    //iterate with iterator to safely remove invalid buttons
    //    for (auto it = m_buttons.begin(); it != m_buttons.end(); ) {
    //        GameObject* btn = it->second;
    //        if (isPointInButton(worldMousePos, btn)) {
    //            Button* btnComp = btn->getComponent<Button>(); //get button component
    //            if (btnComp) {
    //                auto actionIt = m_buttonActions.find(btnComp->actionName); //find action by name
    //                if (actionIt != m_buttonActions.end()) actionIt->second(); //execute action
    //            }
    //            return; //stop checking after a click
    //        }
    //        ++it;
    //    }
    //}

    // on hover
    for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it) {
        GameObject* btnObj = it->second;
        Button* btnComp = btnObj->getComponent<Button>();
        if (!btnComp) continue;

        bool isOver = isPointInButton(worldMousePos, btnObj);

        // hover sound
        if (isOver && !btnComp->isHovered && !btnComp->onHoverChannel.empty()) {
            AudioComponent* audio = btnObj->getComponent<AudioComponent>();
            if (audio) {
                AudioChannel* ch = audio->getChannel(btnComp->onHoverChannel);
                if (ch) ch->isPendingPlay = true;
            }
        }

        // hover exit
        if (!isOver && btnComp->isHovered && !btnComp->onHoverChannel.empty()) {
            AudioComponent* audio = btnObj->getComponent<AudioComponent>();
            if (audio) {
                AudioChannel* ch = audio->getChannel(btnComp->onHoverChannel);
                if (ch) ch->isPendingStop = true;
            }
        }

        btnComp->isHovered = isOver;
    }

    // on audio
    if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        for (auto it = m_buttons.begin(); it != m_buttons.end(); ) {
            GameObject* btnObj = it->second;
            if (isPointInButton(worldMousePos, btnObj)) {
                Button* btnComp = btnObj->getComponent<Button>();
                if (btnComp) {
                    // play click sounda
                    if (!btnComp->onClickChannel.empty()) {
                        AudioComponent* audio = btnObj->getComponent<AudioComponent>();
                        if (audio) {
                            AudioChannel* ch = audio->getChannel(btnComp->onClickChannel);
                            if (ch) {
                                ch->isPendingPlay = true;
                                ch->persistOnSceneChange = true;
                            }
                        }
                    }

                    m_pendingAction = btnComp->actionName; // run action next frame so we can play audio first

                }
                return;
            }
            ++it;
        }
    }
}

//creation of buttons
void GUISystem::createButtons(/*GameObjectManager& manager*/) {
    //clear previous references
    removeButtons();

    //get all game objects from manager
    std::vector<GameObject*> allObjs;
    g_manager->getAllGameObjects(allObjs);

    if (allObjs.empty()) return; //check if its empty

    //iterate through all objects to find buttons
    for (GameObject* obj : allObjs) {
        if (!obj) continue; //safety check
        if (!obj->hasComponent<Button>() || !obj->hasComponent<Transform>()) continue; //check if it has button and transform component

        Button* btnComp = obj->getComponent<Button>(); //get button component
        m_buttons[btnComp->actionName] = obj; //store button in map with action name as key
    }

    if(m_buttons.empty()) {
        std::cout << "[GUI] No buttons found in the current scene." << std::endl;
	}
    else {
        createButtonActions(); //create button actions
        std::cout << "[GUI] Menu buttons created successfully" << std::endl;
    }   

    // Safe to update volume display here, after all buttons/actions are set up
    if (m_currentState == GameState::AUDIO) {
        updateVolumeDisplay();
    }

    // Populate leaderboard text objects when entering the leaderboard screen
    if (m_currentState == GameState::LEADERBOARD) {
        updateLeaderboardDisplay();
    }
}

//removing of buttons 
void GUISystem::removeButtons() {
    m_buttons.clear(); //clear map
}

//to update state
void GUISystem::changeState(GameState newState, const std::string& sceneName) {
    m_previousState = m_currentState;  // store previous
    m_currentState = newState;

    if (!sceneName.empty()) {
        loadScreen(*g_manager, sceneName);
    }
}

//create button actions
void GUISystem::createButtonActions() {
    m_buttonActions.clear();

    //play
    m_buttonActions[ButtonAction::START_GAME] = [this]() {
        changeState(GameState::PLAYING);
        startGame();
     };

    //exit confirmation
    m_buttonActions[ButtonAction::EXIT_GAME] = [this]() {
        changeState(GameState::EXITCONFIRMATION, "confirmation_scene.json");
     };

    //control screen
    m_buttonActions[ButtonAction::OPEN_CONTROL] = [this]() {
        changeState(GameState::CONTROL, "control_scene.json");
     };

    // audio scene
    m_buttonActions[ButtonAction::OPEN_AUDIO] = [this]() {
        changeState(GameState::AUDIO, "audio_scene.json");
        //updateVolumeDisplay();
    };

    m_buttonActions[ButtonAction::BACK] = [this]() {
        switch (m_previousState) {
        case GameState::MENU:
            changeState(GameState::MENU, "menu_scene.json");
            break;
        case GameState::CONTROL:
            changeState(GameState::CONTROL, "control_scene.json");
            break;
        case GameState::AUDIO:
            changeState(GameState::AUDIO, "audio_scene.json");
            break;
        case GameState::PAUSED:
            changeState(GameState::PAUSED, "pause_scene.json");
            break;
        case GameState::PLAYING:
            startGame(); // resume the game
            break;
        case GameState::EXITCONFIRMATION:
            changeState(GameState::EXITCONFIRMATION, "confirmation_scene.json");
            break;
        case GameState::LEADERBOARD:
            changeState(GameState::LEADERBOARD, "leaderboard_scene.json");
            break;
        default:
            changeState(GameState::MENU, "menu_scene.json");
            break;
        }
    };

    //menu button
    m_buttonActions[ButtonAction::MENU] = [this]() {
        // Clean up replay/ghost when leaving gameplay
        auto& replay = ReplaySystem::getInstance();
        replay.discardRecording();
        replay.stopPlayback(*g_manager);

        // Clean up replay viewer if active
        if (m_replayViewerActive) {
            replay.stopViewer();
            m_replayViewerActive = false;
        }

        changeState(GameState::MENU, "menu_scene.json");
    };

    //resume
    m_buttonActions[ButtonAction::RESUME_GAME] = [this]() {
        m_buttonActions[ButtonAction::BACK](); //uses logic from back
    };

    //exit confirmation screen yes/no
    m_buttonActions[ButtonAction::YES_EXIT] = [this]() {
        m_currentState = GameState::EXIT;
        stopGame();
    };

    m_buttonActions[ButtonAction::NO_EXIT] = [this]() {
        m_currentState = GameState::MENU;
        loadScreen(*g_manager, "menu_scene.json");
    };

    // Single master volume controls
    m_buttonActions[ButtonAction::MUSIC_VOL_UP] = [this]() {
        AudioHandler& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() + 0.1f);
        m_previousVolume = ah.getMasterVolume();
        updateVolumeDisplay();
    };
    m_buttonActions[ButtonAction::MUSIC_VOL_DOWN] = [this]() {
        AudioHandler& ah = AudioHandler::getInstance();
        ah.setMasterVolume(ah.getMasterVolume() - 0.1f);
        m_previousVolume = ah.getMasterVolume();
        updateVolumeDisplay();
    };
    //mute
    m_buttonActions[ButtonAction::MUTE] = [this]() {
        AudioHandler& ah = AudioHandler::getInstance();
        m_previousVolume = ah.getMasterVolume();
        ah.setMasterVolume(ah.getMasterVolume() * 0);
        updateVolumeDisplay();
    };
    //unmute
    m_buttonActions[ButtonAction::UNMUTE] = [this]() {
        AudioHandler& ah = AudioHandler::getInstance();
        ah.setMasterVolume(m_previousVolume);
        updateVolumeDisplay();
    };

    // leaderboard screen
    m_buttonActions[ButtonAction::OPEN_LEADERBOARD] = [this]() {
        changeState(GameState::LEADERBOARD, "leaderboard_scene.json");
    };

    // replay viewer
    m_buttonActions[ButtonAction::WATCH_REPLAY] = [this]() {
        startReplayViewer();
    };

    // replay selection screen
    m_buttonActions[ButtonAction::OPEN_REPLAY_SELECT] = [this]() {
        openReplaySelect();
    };

    std::cout << "[GUI] Menu buttons created successfully" << std::endl;
}

void GUISystem::updateVolumeDisplay() {
    AudioHandler& ah = AudioHandler::getInstance();

    // Use a single volume text element (rename in your audio_scene.json accordingly)
    GameObject* volText = g_manager->getGameObject("music_vol_text");
    if (volText && volText->hasComponent<FontComponent>()) {
        int pct = static_cast<int>(ah.getMasterVolume() * 100.0f + 0.5f);
        volText->getComponent<FontComponent>()->word = std::to_string(pct) + "%";
    }
}

void GUISystem::updateLeaderboardDisplay() {
    auto& lb = LeaderboardSystem::getInstance();
    auto entries = lb.getAll();

    for (int i = 0; i < LeaderboardSystem::MAX_ENTRIES; ++i) {
        std::string idx = std::to_string(i + 1);

        GameObject* rankObj = g_manager->getGameObject("lb_rank_" + idx);
        GameObject* nameObj = g_manager->getGameObject("lb_name_" + idx);
        GameObject* timeObj = g_manager->getGameObject("lb_time_" + idx);

        if (i < static_cast<int>(entries.size())) {
            int totalMs = entries[i].score;
            int minutes = totalMs / 60000;
            int seconds = (totalMs % 60000) / 1000;
            int ms = totalMs % 1000;
            char timeBuf[32];
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d.%03d", minutes, seconds, ms);

            if (rankObj && rankObj->hasComponent<FontComponent>())
                rankObj->getComponent<FontComponent>()->word = idx + ".";
            if (nameObj && nameObj->hasComponent<FontComponent>())
                nameObj->getComponent<FontComponent>()->word = entries[i].name;
            if (timeObj && timeObj->hasComponent<FontComponent>())
                timeObj->getComponent<FontComponent>()->word = timeBuf;
        }
        else {
            if (rankObj && rankObj->hasComponent<FontComponent>())
                rankObj->getComponent<FontComponent>()->word = idx + ".";
            if (nameObj && nameObj->hasComponent<FontComponent>())
                nameObj->getComponent<FontComponent>()->word = "---";
            if (timeObj && timeObj->hasComponent<FontComponent>())
                timeObj->getComponent<FontComponent>()->word = "---";
        }
    }
}

void GUISystem::initInGameHUD() {
    auto& lb = LeaderboardSystem::getInstance();
    auto entries = lb.getTopN(3);

    for (int i = 0; i < 3; ++i) {
        std::string idx = std::to_string(i + 1);
        GameObject* scoreObj = g_manager->getGameObject("hud_lb_" + idx);
        if (!scoreObj || !scoreObj->hasComponent<FontComponent>()) continue;

        FontComponent* fc = scoreObj->getComponent<FontComponent>();
        if (i < static_cast<int>(entries.size())) {
            int totalMs = entries[i].score;
            int minutes = totalMs / 60000;
            int seconds = (totalMs % 60000) / 1000;
            int ms = totalMs % 1000;
            char timeBuf[32];
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d.%03d", minutes, seconds, ms);
            fc->word = idx + ". " + entries[i].name + "  " + timeBuf;
        }
        else {
            fc->word = idx + ". ---  ---";
        }
    }
}

void GUISystem::updateInGameHUD() {
    if (m_currentState != GameState::PLAYING) return;

    // Update timer text every frame
    GameObject* timerObj = g_manager->getGameObject("hud_timer");
    if (timerObj && timerObj->hasComponent<FontComponent>()) {
        int totalMs = static_cast<int>(m_gameTimer * 1000.0);
        int minutes = totalMs / 60000;
        int seconds = (totalMs % 60000) / 1000;
        int ms = totalMs % 1000;
        char timeBuf[32];
        std::snprintf(timeBuf, sizeof(timeBuf), "TIME: %d:%02d.%03d", minutes, seconds, ms);
        timerObj->getComponent<FontComponent>()->word = timeBuf;
    }
}

void GUISystem::startGame() {
    std::cout << "[GUI] Starting game" << std::endl;

    //change state to playing
	m_currentState = GameState::PLAYING;
    level = 1;

    // Reset and start the level timer
    m_levelTimer = 0.0;
    m_levelTimerRunning = true;

    // Reset and start the total game timer (runs from start to win)
    m_gameTimer = 0.0;
    m_gameTimerRunning = true;

    // Clear any in-progress replay data for a fresh run
    ReplaySystem::getInstance().clearCurrentRun();

    removeButtons(); //remove menu buttons

	loadScreenNoButton(*g_manager, "level01.json");

    m_currentLevel = std::string(RUNTIME_SCENE_DIR_R) + "/level01.json";

    std::vector<GameObject*> gameObjects;
    g_manager->getAllGameObjects(gameObjects);
    //iterate over all Lua scripts in the folder
    for (const auto& file : std::filesystem::directory_iterator("./assets/scripting")) {
        if (file.path().extension() != ".lua") continue; //have to be .lua files if not it will skip
        std::string filename = file.path().filename().string();

        //strip ".lua" to get object name
        std::string objectName = filename.substr(0, filename.size() - 4);

        GameObject* obj = g_manager->getGameObject(objectName);
        if (obj) {
            obj->addComponent<LuaScript>(file.path().string()); //add LuaScript component to object
            //load Lua script into Lua state
            if (!g_luaSystem->loadScriptForObject(file.path().string(), objectName)) {
                std::cerr << "[Lua] Failed to load script for " << objectName << std::endl;
            }
            else {
                std::cout << "[Lua] Loaded script " << filename << " for object " << objectName << std::endl;
            }
        }
    }
    std::cout << "[GUI] Game started - loaded json" << std::endl;

    // Start replay recording and ghost playback for this level
    auto& replay = ReplaySystem::getInstance();
    replay.startRecording(m_currentLevel);
    replay.startPlayback(m_currentLevel, *g_manager);

    // Populate in-game HUD with leaderboard top 3
    initInGameHUD();
}
void GUISystem::stopGame() {
    std::cout << "[GUI] Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

//check if mouse is pointing the button
bool GUISystem::isPointInButton(const Vector2D& point, GameObject* button) {
    if (!button || !button->hasComponent<Transform>()) return false;

    Transform* transform = button->getComponent<Transform>();

    // Simple AABB collision check for button
    float halfWidth = transform->scaleX * 0.5f;
    float halfHeight = transform->scaleY * 0.5f;

    return (point.x >= transform->x - halfWidth &&
        point.x <= transform->x + halfWidth &&
        point.y >= transform->y - halfHeight &&
        point.y <= transform->y + halfHeight);
}

//buttons
void GUISystem::loadScreen(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Loading screen: " << sceneName << "\n";

    removeButtons(); // clear references
    AudioHandler::getInstance().stopAll(manager);
    manager.clearScene(); //delete al scene object

    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources();

    createButtons(/*manager*/);
}

//no buttons
void GUISystem::loadScreenNoButton(GameObjectManager& manager, const std::string& sceneName) {
    std::cout << "[GUI] Loading screen: " << sceneName << "\n";

    AudioHandler::getInstance().stopAll(manager);
    manager.clearScene(); //delete al scene object
    manager.init();
    manager.loadScene(runtimeScenePath(sceneName));
    manager.initializeSceneResources();
}


//helper function to convert scene name to game state
GameState GUISystem::sceneToState(const std::string& sceneName) {
    if (sceneName == "menu_scene.json") {
        return GameState::MENU;
    }
    if (sceneName == "control_scene.json") {
        return GameState::CONTROL;
    }
    if (sceneName == "audio_scene.json") {
        return GameState::AUDIO;
    }
    if (sceneName == "level01.json" || sceneName == "level02.json") {
		//if (!m_buttons.empty()) removeButtons();
        return GameState::PLAYING;
    }
    if (sceneName == "pause_scene.json") {
        return GameState::PAUSED;
    }
    if (sceneName == "confirmation_scene.json") {
        return GameState::EXITCONFIRMATION;
    }
    if (sceneName == "leaderboard_scene.json") {
        return GameState::LEADERBOARD;
    }
    if (sceneName == "replay_select_scene.json") {
        return GameState::REPLAY;
    }
    return GameState::MENU; //fallback
}

void GUISystem::restartLevel() {
    std::cout << "[GUI] Restarting level" << std::endl;

    if (m_currentLevel.empty()) {
        std::cerr << "[GUI] No current level to restart!" << std::endl;
        return;
    }

    // Stop any in-progress replay recording/playback before clearing scene
    auto& replay = ReplaySystem::getInstance();
    replay.discardRecording();
    replay.stopPlayback(*g_manager);

    removeButtons();
    AudioHandler::getInstance().stopAll(*g_manager);

    g_manager->clearScene();
    g_manager->loadScene(m_currentLevel);
    g_manager->initializeSceneResources();
    g_visualLogicSystem->resetLogicState();
    g_visualLogicSystem->initAll(*g_manager);
    resetAllObjectsToSpawn();
    m_currentState = GameState::PLAYING;

    // Reset and start the level timer
    m_levelTimer = 0.0;
    m_levelTimerRunning = true;

    // Start fresh recording and ghost playback
    replay.startRecording(m_currentLevel);
    replay.startPlayback(m_currentLevel, *g_manager);

    // Populate in-game HUD with leaderboard top 3
    initInGameHUD();
}

void GUISystem::loadLevel(const std::string& levelName) {
    std::cout << "[GUI] Loading level: " << levelName << std::endl;

    std::string fullPath = std::string(RUNTIME_SCENE_DIR_R) + "/" + levelName;
    m_currentLevel = fullPath;

    //m_currentLevel = levelName;
    removeButtons();
    AudioHandler::getInstance().stopAll(*g_manager);

    g_manager->clearScene();

    g_manager->loadScene(fullPath);
    g_manager->initializeSceneResources();
    g_visualLogicSystem->resetLogicState();
    g_visualLogicSystem->initAll(*g_manager);

    // Save spawn positions for this level
    m_spawnPositions.clear();
    std::vector<GameObject*> allObjects;
    g_manager->getAllGameObjects(allObjects);

    for (GameObject* obj : allObjects) {
        if (obj->hasComponent<Transform>()) {
            Transform* transform = obj->getComponent<Transform>();
            m_spawnPositions[obj->getObjectName()] = Vector2D(transform->x, transform->y);
        }
    }

    m_currentState = GameState::PLAYING;

    // Reset and start the level timer
    m_levelTimer = 0.0;
    m_levelTimerRunning = true;

    // Start replay recording and ghost playback for this level
    auto& replay = ReplaySystem::getInstance();
    replay.startRecording(m_currentLevel);
    replay.startPlayback(m_currentLevel, *g_manager);

    // Populate in-game HUD with leaderboard top 3
    initInGameHUD();
}

void GUISystem::startReplayViewer(const std::string& requestedRunId) {
    auto& replay = ReplaySystem::getInstance();

    // Use provided run ID, or fall back to last completed run, or best run
    std::string runId = requestedRunId;
    if (runId.empty()) runId = m_lastRunId;
    if (runId.empty()) runId = replay.findBestRunId();

    if (runId.empty()) {
        std::cout << "[GUI] No completed runs available to watch.\n";
        return;
    }

    m_replayRunId = runId;
    std::cout << "[GUI] Starting replay viewer for run " << runId << "ms\n";

    // Build ordered list of levels that have replays in this run
    m_replayLevels.clear();
    const std::vector<std::string> allLevels = {
        "level01", "level02", "level03", "level04"
    };
    for (const auto& lvl : allLevels) {
        if (replay.hasReplayForRun(lvl, runId)) {
            m_replayLevels.push_back(lvl);
        }
    }

    if (m_replayLevels.empty()) {
        std::cout << "[GUI] No replays in run " << runId << ".\n";
        return;
    }

    m_replayLevelIndex = 0;
    m_replayViewerActive = true;
    m_currentState = GameState::REPLAY;
    loadReplayLevel(0);
}

void GUISystem::loadReplayLevel(int levelIndex) {
    const std::string& levelName = m_replayLevels[levelIndex];
    std::string sceneName = levelName + ".json";
    std::string fullPath = std::string(RUNTIME_SCENE_DIR_R) + "/" + sceneName;

    std::cout << "[GUI] Replay viewer: loading " << levelName << "\n";

    removeButtons();
    AudioHandler::getInstance().stopAll(*g_manager);

    g_manager->clearScene();
    g_manager->loadScene(fullPath);
    g_manager->initializeSceneResources();
    g_visualLogicSystem->resetLogicState();
    g_visualLogicSystem->initAll(*g_manager);

    // Initialize Lua scripts for this level
    for (const auto& file : std::filesystem::directory_iterator("./assets/scripting")) {
        if (file.path().extension() != ".lua") continue;
        std::string filename = file.path().filename().string();
        std::string objectName = filename.substr(0, filename.size() - 4);
        GameObject* obj = g_manager->getGameObject(objectName);
        if (obj) {
            obj->addComponent<LuaScript>(file.path().string());
            g_luaSystem->loadScriptForObject(file.path().string(), objectName);
        }
    }

    // Start viewer playback (feeds recorded inputs, no ghost spawned)
    auto& replay = ReplaySystem::getInstance();
    replay.startViewerPlayback(fullPath, m_replayRunId);
}

void GUISystem::tickLevelTimer(double dt) {
    if (m_levelTimerRunning && m_currentState == GameState::PLAYING) {
        m_levelTimer += dt;
    }
    if (m_gameTimerRunning && m_currentState == GameState::PLAYING) {
        m_gameTimer += dt;
    }
}

void GUISystem::resetAllObjectsToSpawn() {
    std::cout << "[GUI] Resetting all objects to spawn positions" << std::endl;

    std::vector<GameObject*> allObjects;
    g_manager->getAllGameObjects(allObjects);

    for (GameObject* obj : allObjects) {
        std::string objName = obj->getObjectName();

        // Reset position to spawn
        if (m_spawnPositions.count(objName)) {
            Transform* transform = obj->getComponent<Transform>();
            if (transform) {
                Vector2D spawnPos = m_spawnPositions[objName];
                transform->x = spawnPos.x;
                transform->y = spawnPos.y;
            }
        }

        if (obj->hasComponent<AudioComponent>()) {
			for (auto& soundPair : obj->getComponent<AudioComponent>()->audioChannels) {
				soundPair.second.state = AudioState::Stopped;
			}
        }

        // Reset velocity to zero
        if (obj->hasComponent<Physics>()) {
            Physics* physics = obj->getComponent<Physics>();
            physics->velX = 0.0f;
            physics->velY = 0.0f;
        }

        // Reset to idle state
        if (obj->hasComponent<StateMachine>()) {
            StateMachine* sm = obj->getComponent<StateMachine>();
            sm->state = PlayerState::Idle;
			sm->facingRight = true;
            sm->fixState = false;
			sm->stateTime = 0.f;

			if (obj->hasComponent<Animation>()) {
				Animation* anim = obj->getComponent<Animation>();
                anim->animState[static_cast<int>(sm->state)].currentFrameColumn = static_cast<int>(anim->animState[static_cast<int>(sm->state)].initialFrame.x);
                anim->animState[static_cast<int>(sm->state)].currentFrameRow = static_cast<int>(anim->animState[static_cast<int>(sm->state)].initialFrame.y);
			}
        }

        if (obj->hasComponent<TileMap>()) {

			TileMap* tileMap = obj->getComponent<TileMap>();
            tileMap->collisionBoxs.clear();
			tileMap->colChanged = true;

            if (tileMap->enableCol && obj->hasComponent<Transform>()) {
                Transform* tmTransform = obj->getComponent<Transform>();
                Collision::buildTileCollisionBox(tileMap, tmTransform);
            }
        }
    }
}

// ============================================================================
// Replay Selection Screen
// ============================================================================

void GUISystem::openReplaySelect() {
    auto& replay = ReplaySystem::getInstance();
    m_availableRuns = replay.getAllRunIds();
    m_replaySelectIndex = 0;
    m_replaySelectPage = 0;
    m_replaySelectActive = true;
    m_replayViewerActive = false;

    changeState(GameState::REPLAY, "replay_select_scene.json");
    populateReplaySelection();
}

void GUISystem::populateReplaySelection() {
    const int entriesPerPage = 5;
    int startIdx = m_replaySelectPage * entriesPerPage;
    int totalRuns = static_cast<int>(m_availableRuns.size());

    for (int i = 0; i < entriesPerPage; ++i) {
        std::string objName = "replay_entry_" + std::to_string(i + 1);
        GameObject* obj = g_manager->getGameObject(objName);
        if (!obj || !obj->hasComponent<FontComponent>()) continue;
        FontComponent* fc = obj->getComponent<FontComponent>();

        int runIdx = startIdx + i;
        if (runIdx < totalRuns) {
            const std::string& runId = m_availableRuns[runIdx];
            int timeMs = 0;
            try { timeMs = std::stoi(runId); } catch (...) {}
            int minutes = timeMs / 60000;
            int seconds = (timeMs % 60000) / 1000;
            int ms = timeMs % 1000;
            char timeBuf[32];
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d.%03d", minutes, seconds, ms);
            fc->word = "Run " + std::to_string(runIdx + 1) + " - " + std::string(timeBuf);

            // Highlight selected entry
            if (runIdx == m_replaySelectIndex) {
                fc->clr = glm::vec3(1.0f, 0.84f, 0.0f); // gold
            } else {
                fc->clr = glm::vec3(1.0f, 1.0f, 1.0f); // white
            }
        } else {
            fc->word = "---";
            fc->clr = glm::vec3(0.4f, 0.4f, 0.4f);
        }
    }

    // Update selector arrow position
    GameObject* selector = g_manager->getGameObject("replay_selector");
    if (selector && selector->hasComponent<Transform>()) {
        int localIdx = m_replaySelectIndex - startIdx;
        if (localIdx >= 0 && localIdx < entriesPerPage) {
            float yPos = 4.5f - localIdx * 2.0f;
            Transform* t = selector->getComponent<Transform>();
            t->y = yPos;
        }
    }

    // Update page info
    int totalPages = (totalRuns + entriesPerPage - 1) / entriesPerPage;
    if (totalPages < 1) totalPages = 1;
    GameObject* pageObj = g_manager->getGameObject("replay_page_info");
    if (pageObj && pageObj->hasComponent<FontComponent>()) {
        FontComponent* fc = pageObj->getComponent<FontComponent>();
        if (totalRuns == 0) {
            fc->word = "No replays found";
        } else {
            fc->word = "Page " + std::to_string(m_replaySelectPage + 1) + "/" + std::to_string(totalPages)
                     + "  (" + std::to_string(totalRuns) + " runs)";
        }
    }
}

void GUISystem::handleReplaySelectInput() {
    const int entriesPerPage = 5;
    int totalRuns = static_cast<int>(m_availableRuns.size());

    // ESC goes back to menu
    if (InputHandler::isKeyTriggered(GLFW_KEY_ESCAPE)) {
        m_replaySelectActive = false;
        changeState(GameState::MENU, "menu_scene.json");
        return;
    }

    if (totalRuns == 0) return;

    bool changed = false;

    // UP/DOWN navigate entries
    if (InputHandler::isKeyTriggered(GLFW_KEY_UP)) {
        if (m_replaySelectIndex > 0) {
            m_replaySelectIndex--;
            int page = m_replaySelectIndex / entriesPerPage;
            if (page != m_replaySelectPage) m_replaySelectPage = page;
            changed = true;
        }
    }
    if (InputHandler::isKeyTriggered(GLFW_KEY_DOWN)) {
        if (m_replaySelectIndex < totalRuns - 1) {
            m_replaySelectIndex++;
            int page = m_replaySelectIndex / entriesPerPage;
            if (page != m_replaySelectPage) m_replaySelectPage = page;
            changed = true;
        }
    }

    // LEFT/RIGHT change pages
    if (InputHandler::isKeyTriggered(GLFW_KEY_LEFT)) {
        if (m_replaySelectPage > 0) {
            m_replaySelectPage--;
            m_replaySelectIndex = m_replaySelectPage * entriesPerPage;
            changed = true;
        }
    }
    if (InputHandler::isKeyTriggered(GLFW_KEY_RIGHT)) {
        int totalPages = (totalRuns + entriesPerPage - 1) / entriesPerPage;
        if (m_replaySelectPage < totalPages - 1) {
            m_replaySelectPage++;
            m_replaySelectIndex = m_replaySelectPage * entriesPerPage;
            changed = true;
        }
    }

    if (changed) populateReplaySelection();

    // ENTER selects replay and starts viewer
    if (InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
        if (m_replaySelectIndex >= 0 && m_replaySelectIndex < totalRuns) {
            m_replaySelectActive = false;
            startReplayViewer(m_availableRuns[m_replaySelectIndex]);
        }
    }
}

// ============================================================================
// Replay Viewer Playback Controls
// ============================================================================

void GUISystem::handleReplayViewerControls() {
    auto& replay = ReplaySystem::getInstance();

    // F2: Toggle pause/resume
    if (InputHandler::isKeyTriggered(GLFW_KEY_F2)) {
        replay.toggleViewerPause();
    }

    // LEFT arrow: Seek backward 2 seconds
    if (InputHandler::isKeyTriggered(GLFW_KEY_LEFT)) {
        replay.seekViewer(-2.0f);
    }

    // RIGHT arrow: Seek forward 2 seconds
    if (InputHandler::isKeyTriggered(GLFW_KEY_RIGHT)) {
        replay.seekViewer(2.0f);
    }

    // Comma: Step back 1 frame (when paused)
    if (InputHandler::isKeyTriggered(GLFW_KEY_COMMA) && replay.isViewerPaused()) {
        replay.stepViewerFrame(-1);
    }

    // Period: Step forward 1 frame (when paused)
    if (InputHandler::isKeyTriggered(GLFW_KEY_PERIOD) && replay.isViewerPaused()) {
        replay.stepViewerFrame(1);
    }

    // Minus: Decrease speed (cycle: 2.0 -> 1.0 -> 0.5 -> 0.25)
    if (InputHandler::isKeyTriggered(GLFW_KEY_MINUS)) {
        float spd = replay.getViewerSpeed();
        if (spd > 1.0f) spd = 1.0f;
        else if (spd > 0.5f) spd = 0.5f;
        else spd = 0.25f;
        replay.setViewerSpeed(spd);
    }

    // Equals/Plus: Increase speed (cycle: 0.25 -> 0.5 -> 1.0 -> 2.0)
    if (InputHandler::isKeyTriggered(GLFW_KEY_EQUAL)) {
        float spd = replay.getViewerSpeed();
        if (spd < 0.5f) spd = 0.5f;
        else if (spd < 1.0f) spd = 1.0f;
        else spd = 2.0f;
        replay.setViewerSpeed(spd);
    }

    // N: Skip to next level in the replay
    if (InputHandler::isKeyTriggered(GLFW_KEY_N)) {
        replay.stopViewer();
        m_replayLevelIndex++;
        if (m_replayLevelIndex < static_cast<int>(m_replayLevels.size())) {
            loadReplayLevel(m_replayLevelIndex);
        } else {
            m_replayViewerActive = false;
            changeState(GameState::MENU, "menu_scene.json");
        }
    }
}

void GUISystem::updateReplayHUD() {
    auto& replay = ReplaySystem::getInstance();

    // Update hud_timer to show replay progress
    GameObject* timerObj = g_manager->getGameObject("hud_timer");
    if (timerObj && timerObj->hasComponent<FontComponent>()) {
        FontComponent* fc = timerObj->getComponent<FontComponent>();
        float elapsed = replay.getViewerElapsed();
        float duration = replay.getViewerDuration();
        int eMin = static_cast<int>(elapsed) / 60;
        int eSec = static_cast<int>(elapsed) % 60;
        int eMs  = static_cast<int>(elapsed * 1000) % 1000;
        int dMin = static_cast<int>(duration) / 60;
        int dSec = static_cast<int>(duration) % 60;
        int dMs  = static_cast<int>(duration * 1000) % 1000;

        char buf[96];
        std::snprintf(buf, sizeof(buf), "[REPLAY] %d:%02d.%03d / %d:%02d.%03d",
                       eMin, eSec, eMs, dMin, dSec, dMs);

        std::string status(buf);
        if (replay.isViewerPaused()) status += "  PAUSED";
        float spd = replay.getViewerSpeed();
        if (spd != 1.0f) {
            char spdBuf[16];
            std::snprintf(spdBuf, sizeof(spdBuf), "  %.2fx", spd);
            status += spdBuf;
        }
        fc->word = status;
    }

    // Show controls in leaderboard header area
    GameObject* headerObj = g_manager->getGameObject("hud_lb_header");
    if (headerObj && headerObj->hasComponent<FontComponent>()) {
        FontComponent* fc = headerObj->getComponent<FontComponent>();
        fc->word = "F2:Pause </>:Seek -/+:Speed";
    }

    // Show level info and frame count
    GameObject* lb1Obj = g_manager->getGameObject("hud_lb_1");
    if (lb1Obj && lb1Obj->hasComponent<FontComponent>()) {
        FontComponent* fc = lb1Obj->getComponent<FontComponent>();
        std::string lvl = (m_replayLevelIndex < static_cast<int>(m_replayLevels.size()))
            ? m_replayLevels[m_replayLevelIndex] : "???";
        fc->word = "Level: " + lvl + " (" + std::to_string(m_replayLevelIndex + 1)
                 + "/" + std::to_string(m_replayLevels.size()) + ")";
    }

    GameObject* lb2Obj = g_manager->getGameObject("hud_lb_2");
    if (lb2Obj && lb2Obj->hasComponent<FontComponent>()) {
        FontComponent* fc = lb2Obj->getComponent<FontComponent>();
        fc->word = "Frame: " + std::to_string(replay.getViewerFrameIndex())
                 + "/" + std::to_string(replay.getViewerFrameCount());
    }

    GameObject* lb3Obj = g_manager->getGameObject("hud_lb_3");
    if (lb3Obj && lb3Obj->hasComponent<FontComponent>()) {
        FontComponent* fc = lb3Obj->getComponent<FontComponent>();
        fc->word = ",/.:FrameStep  N:NextLvl  ESC:Exit";
    }
}