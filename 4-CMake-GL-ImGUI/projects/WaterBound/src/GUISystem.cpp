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

    // Replay viewer: feed recorded inputs through the game simulation
    if (m_replayViewerActive && m_currentState == GameState::REPLAY) {
        auto& replay = ReplaySystem::getInstance();

        // When the viewer finishes the current level, advance to the next
        if (replay.isViewerFinished()) {
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

        // ESC exits replay viewer
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
}

//stop game and quit
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
}

void GUISystem::startReplayViewer() {
    auto& replay = ReplaySystem::getInstance();

    // Build ordered list of levels that have saved replays
    m_replayLevels.clear();
    const std::vector<std::string> allLevels = {
        "level01", "level02", "level03", "level04"
    };
    for (const auto& lvl : allLevels) {
        if (replay.hasReplay(lvl)) {
            m_replayLevels.push_back(lvl);
        }
    }

    if (m_replayLevels.empty()) {
        std::cout << "[GUI] No replays available to watch.\n";
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
    replay.startViewerPlayback(fullPath);
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