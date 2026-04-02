/* Start Header ************************************************************************/
/*!
\file       SceneManager.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591 - 50%
            Tan Chee Qing, cheeqing.tan, 2401486 - 50%
\par        p.lin@digipen.edu
\date       March, 27th, 2025
\brief      Implementation for the SceneManager class, which handles scene loading,
            restarting, pausing, resuming and history management for back button.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include <iostream>
#include <filesystem>
#include "SceneManager.h"
#include "Systems.h"       // VisualLogicSystem
#include "audio.h"         // AudioHandler
#include "CoreEngine.h"    // RUNTIME_SCENE_DIR_R
#include "collision.h"     // Collision::buildTileCollisionBox
#include "ButtonSystem.h"

/*static*/ std::string SceneManager::buildScenePath(const std::string& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

void SceneManager::init(GameObjectManager& manager, VisualLogicSystem& vlSystem) {
    g_manager = &manager;
    g_visualLogicSystem = &vlSystem;

    std::cout << "[SceneManager] init complete\n";
}

void SceneManager::update() {
    if (m_pendingRestart) {
        m_pendingRestart = false;
        executeRestart();
        return;
    }

    if (m_pendingResume) {
        m_pendingResume = false;
        executeResume();
        return;
    }

    if (m_pendingLoad && !UISystem::isShowUI()) {
        m_pendingLoad = false;
        std::string name = std::move(m_pendingLoadName);
        executeLoad(name);
    }
}

void SceneManager::requestLoadScene(const std::string& sceneName, bool pushHistory) {
    m_pendingLoad = true;
    m_pendingLoadName = sceneName;
    m_pendingPushHistory = pushHistory;
}

void SceneManager::requestPause(const std::string& pauseSceneName) {
    // snapshot current scene into memory
    m_pauseSnapshot = JsonIO::SceneToString(*g_manager);
    if (m_pauseSnapshot.empty()) {
        std::cerr << "[SceneManager] Failed to snapshot scene for pause\n";
        return;
    }

    m_pausedFromScene = m_currentSceneName;
    m_isPaused = true;
	m_sceneHistory.push(m_currentSceneName);
    requestLoadScene(pauseSceneName, false); // no history push
}

void SceneManager::popHistory() {
    if (!m_sceneHistory.empty())
        m_sceneHistory.pop();
}

void SceneManager::requestRestart() {
    m_pendingRestart = true;
}

void SceneManager::loadScene(const std::string& sceneName) {
    executeLoad(sceneName);
}

void SceneManager::pauseScene() {
    m_isPaused = true;
}

void SceneManager::resumeScene() {
    m_isPaused = false;
}

void SceneManager::restartScene() {
    requestRestart();
}

void SceneManager::back() {
    if (m_sceneHistory.empty()) return;

    std::string prev = m_sceneHistory.top();
    m_sceneHistory.pop();

    if (m_isPaused && prev == m_pausedFromScene) {
        //m_isPaused = false;
        m_pendingResume = true;
    }
    else {
        // normal navigation outside of pause
        requestLoadScene(prev, false);
    }
}

void SceneManager::executeLoad(const std::string& sceneName) {
    std::cout << "[SceneManager] Loading: " << sceneName << "\n";

    std::string fullPath = buildScenePath(sceneName);

    if (m_pendingPushHistory && !m_currentSceneName.empty())
        m_sceneHistory.push(m_currentSceneName);
    m_pendingPushHistory = true;
    m_currentSceneName = sceneName;
    m_currentSceneFullPath = fullPath;

    AudioHandler::getInstance().stopAll(*g_manager);
    g_manager->clearScene();
    g_manager->loadScene(fullPath);
    g_manager->initializeSceneResources();

    if (g_visualLogicSystem) {
        g_visualLogicSystem->resetLogicState();
        g_visualLogicSystem->initAll(*g_manager);
    }

    captureSpawnPositions();
    ButtonSystem::getInstance().refresh();

    if (m_onSceneLoad) m_onSceneLoad(*g_manager);
}

void SceneManager::executeRestart() {
    std::string sceneToRestart = m_isPaused ? m_pausedFromScene : m_currentSceneName;
    std::string pathToRestart = buildScenePath(sceneToRestart);

    if (pathToRestart.empty()) {
        std::cerr << "[SceneManager] Nothing to restart\n";
        return;
    }

    std::cout << "[SceneManager] Restarting: " << m_currentSceneName << "\n";

    clearPauseState();

    if (m_currentSceneFullPath.empty()) {
        std::cerr << "[SceneManager] Nothing to restart\n";
        return;
    }

    AudioHandler::getInstance().stopAll(*g_manager);
    g_manager->clearScene();
    g_manager->loadScene(pathToRestart);
    g_manager->initializeSceneResources();

    if (g_visualLogicSystem) {
        g_visualLogicSystem->resetLogicState();
        g_visualLogicSystem->initAll(*g_manager);
    }

    m_currentSceneName = sceneToRestart;
    m_currentSceneFullPath = pathToRestart;

    resetAllObjectsToSpawn();
    ButtonSystem::getInstance().refresh();

    if (m_onSceneLoad) m_onSceneLoad(*g_manager);
}

void SceneManager::captureSpawnPositions() {
    m_spawnPositions.clear();
    std::vector<GameObject*> all;
    g_manager->getAllGameObjects(all);

    for (GameObject* obj : all) {
        if (obj->hasComponent<Transform>()) {
            Transform* t = obj->getComponent<Transform>();
            m_spawnPositions[obj->getObjectName()] = Vector2D(t->x, t->y);
        }
    }
}

void SceneManager::resetAllObjectsToSpawn() {

    std::vector<GameObject*> all;
    g_manager->getAllGameObjects(all);

    for (GameObject* obj : all) {
        const std::string& name = obj->getObjectName();

        // --- position ---
        if (m_spawnPositions.count(name)) {
            if (Transform* t = obj->getComponent<Transform>()) {
                t->x = m_spawnPositions[name].x;
                t->y = m_spawnPositions[name].y;
            }
        }

        // --- audio ---
        if (obj->hasComponent<AudioComponent>()) {
            for (auto& pair : obj->getComponent<AudioComponent>()->audioChannels)
                pair.second.state = AudioState::Stopped;
        }

        // --- physics ---
        if (Physics* phys = obj->getComponent<Physics>()) {
            phys->velX = 0.f;
            phys->velY = 0.f;
        }

        // --- state machine ---
        if (StateMachine* sm = obj->getComponent<StateMachine>()) {
            sm->state = PlayerState::Idle;
            sm->facingRight = true;
            sm->fixState = false;
            sm->stateTime = 0.f;

            if (Animation* anim = obj->getComponent<Animation>()) {
                int idx = static_cast<int>(sm->state);
                anim->animState[idx].currentFrameColumn =
                    static_cast<int>(anim->animState[idx].initialFrame.x);
                anim->animState[idx].currentFrameRow =
                    static_cast<int>(anim->animState[idx].initialFrame.y);
            }
        }

        // --- tile map ---
        if (TileMap* tm = obj->getComponent<TileMap>()) {
            tm->collisionBoxs.clear();
            tm->colChanged = true;

            if (tm->enableCol) {
                if (Transform* t = obj->getComponent<Transform>())
                    Collision::buildTileCollisionBox(tm, t);
            }
        }
    }
}

void SceneManager::executeResume() {
    AudioHandler::getInstance().stopAll(*g_manager);
    g_manager->clearScene();
    
    std::string err;
    JsonIO::StringToScene(*g_manager, m_pauseSnapshot, &err);
    if (!err.empty()) {
		std::cerr << "[SceneManager] Failed to restore scene from pause snapshot: " << err << "\n";
		return;
    }

    g_manager->initializeSceneResources();

    if (g_visualLogicSystem) {
        g_visualLogicSystem->resetLogicState();
        g_visualLogicSystem->initAll(*g_manager);
    }

    m_currentSceneName = m_pausedFromScene;
    m_currentSceneFullPath = buildScenePath(m_pausedFromScene);
    m_pausedFromScene.clear();
    m_pauseSnapshot.clear();
    m_isPaused = false;

    captureSpawnPositions();
    ButtonSystem::getInstance().refresh();
    //if (m_onSceneLoad) m_onSceneLoad(*g_manager);
}

void SceneManager::triggerSceneLoad(GameObjectManager& manager) {
    if (m_onSceneLoad) m_onSceneLoad(manager);
}

void SceneManager::resetAndInitVisualLogic(GameObjectManager& manager) {
    if (g_visualLogicSystem) {
        VisualLC::globalFloatVar.clear();
        g_visualLogicSystem->initAll(manager);
    }
}

void SceneManager::clearPauseState() {
    m_isPaused = false;
    m_pausedFromScene.clear();
    m_pauseSnapshot.clear();
}