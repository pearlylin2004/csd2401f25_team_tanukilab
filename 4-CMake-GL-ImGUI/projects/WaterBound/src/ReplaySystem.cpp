/* Start Header ************************************************************************/
/*!
\file       ReplaySystem.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       March, 11th, 2026
\brief      Implementation of the ReplaySystem singleton.
            Records player state each frame, saves/loads replay files using
            RapidJSON, and drives a semi-transparent ghost entity during
            playback.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ReplaySystem.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Component.h"
#include "JsonIO.h"
#include "input.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

#ifndef RUNTIME_ASSETS_DIR_R
#define RUNTIME_ASSETS_DIR_R "./assets"
#endif

// ============================================================================
// Path helpers
// ============================================================================

std::string ReplaySystem::getReplayDir() const {
    return std::string(RUNTIME_ASSETS_DIR_R) + "/replays";
}

std::string ReplaySystem::getRunDir(const std::string& runId) const {
    return getReplayDir() + "/" + runId;
}

std::string ReplaySystem::getReplayPath(const std::string& levelName,
                                         const std::string& runId) const {
    return getRunDir(runId) + "/" + levelName + ".json";
}

std::string ReplaySystem::extractLevelName(const std::string& fullPath) {
    // Strip directory: keep only filename
    std::string name = fullPath;
    auto pos = name.find_last_of("/\\");
    if (pos != std::string::npos) name = name.substr(pos + 1);
    // Strip .json extension
    auto dot = name.rfind(".json");
    if (dot != std::string::npos) name = name.substr(0, dot);
    return name;
}

// ============================================================================
// Init
// ============================================================================

void ReplaySystem::init() {
    std::filesystem::create_directories(getRunDir("current"));
    m_initialized = true;
    std::cout << "[Replay] Initialized — replay dir: " << getReplayDir() << "\n";
}

// ============================================================================
// Recording
// ============================================================================

void ReplaySystem::startRecording(const std::string& levelName) {
    m_currentRecording = ReplayData{};
    m_currentRecording.levelName = extractLevelName(levelName);
    m_recordingElapsed = 0.f;
    m_isRecording = true;
    std::cout << "[Replay] Recording started for level: " << m_currentRecording.levelName << "\n";
}

void ReplaySystem::recordFrame(GameObjectManager& manager, float deltaTime) {
    if (!m_isRecording) return;

    m_recordingElapsed += deltaTime;

    // Find the player entity
    GameObject* player = manager.getGameObject("player");
    if (!player) return;

    Transform* transform = player->getComponent<Transform>();
    if (!transform) return;

    ReplayFrame frame;
    frame.timestamp = m_recordingElapsed;
    frame.x = transform->x;
    frame.y = transform->y;
    frame.scaleX = transform->scaleX;
    frame.scaleY = transform->scaleY;
    frame.rotation = transform->rotation;
    frame.flipX = transform->flipX;

    // Capture player state if available
    StateMachine* sm = player->getComponent<StateMachine>();
    if (sm) {
        frame.animState = static_cast<int>(sm->state);
        frame.facingRight = sm->facingRight;
    }

    // Capture input state for full replay viewer
    // Suppress mouse input during a brief grace period at the start of
    // recording to prevent the UI button click that triggered gameplay
    // from bleeding into the first recorded frames.
    constexpr float kInputGracePeriod = 0.15f;
    bool suppressMouse = (m_recordingElapsed < kInputGracePeriod);

    frame.inputKeyA = InputHandler::isKeyHeld(GLFW_KEY_A);
    frame.inputKeyD = InputHandler::isKeyHeld(GLFW_KEY_D);
    frame.inputSpaceTriggered = InputHandler::isKeyTriggered(GLFW_KEY_SPACE);
    frame.inputSpaceReleased = InputHandler::isKeyReleased(GLFW_KEY_SPACE);
    frame.inputMouseLeftHeld = suppressMouse ? false : InputHandler::isMouseLeftHeld();
    Vector2D mousePos = InputHandler::getWorldMousePosition();
    frame.inputMouseX = static_cast<float>(mousePos.x);
    frame.inputMouseY = static_cast<float>(mousePos.y);

    // Capture world object state (fire, etc.)
    recordWorldObjects(manager, frame);

    m_currentRecording.frames.push_back(frame);
}

void ReplaySystem::recordWorldObjects(GameObjectManager& manager, ReplayFrame& frame) {
    std::vector<GameObject*> allObjs;
    manager.getAllGameObjects(allObjs);

    for (GameObject* obj : allObjs) {
        if (!obj) continue;
        CollisionInfo* col = obj->getComponent<CollisionInfo>();
        if (!col) continue;
        // Capture fire objects (active or extinguished)
        if (col->collisionTag != "fire" && col->collisionTag != "steam") continue;

        WorldObjectSnapshot snap;
        snap.objectName = obj->getObjectName();
        snap.collisionTag = col->collisionTag;
        snap.collisionFlag = col->collisionFlag;
        if (StateMachine* sm = obj->getComponent<StateMachine>())
            snap.smState = static_cast<int>(sm->state);
        if (Render* r = obj->getComponent<Render>())
            snap.alpha = r->clr.a;
        if (Transform* t = obj->getComponent<Transform>()) {
            snap.posX = t->x;
            snap.posY = t->y;
        }
        frame.worldObjects.push_back(snap);
    }
}

void ReplaySystem::stopRecording(int completionTimeMs) {
    if (!m_isRecording) return;

    m_isRecording = false;
    m_currentRecording.completionTimeMs = completionTimeMs;

    std::cout << "[Replay] Recording stopped — " << m_currentRecording.frames.size()
              << " frames, " << completionTimeMs << "ms\n";

    // Always save the latest run so the replay viewer shows the most recent attempt
    saveReplay(m_currentRecording);
    std::cout << "[Replay] Saved replay for " << m_currentRecording.levelName << ".\n";
}

void ReplaySystem::discardRecording() {
    if (m_isRecording) {
        m_isRecording = false;
        m_currentRecording = ReplayData{};
        std::cout << "[Replay] Recording discarded\n";
    }
}

// ============================================================================
// Playback & Ghost
// ============================================================================

void ReplaySystem::startPlayback(const std::string& levelName, GameObjectManager& manager) {
    std::string shortName = extractLevelName(levelName);

    // Load from the best completed run (not current/)
    std::string bestRun = findBestRunId();
    if (bestRun.empty() || !loadReplay(shortName, m_playbackData, bestRun)) {
        std::cout << "[Replay] No replay found for " << shortName << " — no ghost.\n";
        return;
    }

    if (m_playbackData.frames.empty()) {
        std::cout << "[Replay] Replay has no frames — no ghost.\n";
        return;
    }

    m_playbackFrame = 0;
    m_playbackElapsed = 0.f;

    // Remove existing ghost if any
    GameObject* existingGhost = manager.getGameObject(GHOST_OBJECT_NAME);
    if (existingGhost) {
        manager.deleteGameObject(existingGhost);
    }

    // Spawn ghost entity
    GameObject* ghost = manager.createGameObject(GHOST_OBJECT_NAME);
    if (!ghost) {
        std::cerr << "[Replay] Failed to create ghost object\n";
        return;
    }

    // Attach GhostComponent marker so other systems can identify/skip it
    ghost->addComponent<GhostComponent>();

    // Copy the player's Render component to get the same texture/sprite
    GameObject* player = manager.getGameObject("player");
    if (player && player->hasComponent<Render>()) {
        Render* playerRender = player->getComponent<Render>();
        Render* ghostRender = ghost->addComponent<Render>();
        ghostRender->visible = true;
        ghostRender->hasTex = playerRender->hasTex;
        ghostRender->texFile = playerRender->texFile;
        ghostRender->texHDL = playerRender->texHDL;
        ghostRender->hasAnimation = playerRender->hasAnimation;
        ghostRender->modelRef = playerRender->modelRef;
        // Semi-transparent blue-white tint
        ghostRender->clr = glm::vec4(0.5f, 0.7f, 1.0f, 0.35f);
        ghostRender->isTransparent = true;
    }

    // Add Transform from first frame
    const ReplayFrame& first = m_playbackData.frames[0];
    Transform* t = ghost->addComponent<Transform>(first.x, first.y, 0.f, first.rotation,
                                                   first.scaleX, first.scaleY, 0.f);
    t->flipX = first.flipX;

    // Copy Animation component for animated ghost
    if (player && player->hasComponent<Animation>()) {
        Animation* playerAnim = player->getComponent<Animation>();
        Animation* ghostAnim = ghost->addComponent<Animation>();
        ghostAnim->animState = playerAnim->animState;
        ghostAnim->runItBack = playerAnim->runItBack;
    }

    // Copy StateMachine for animation state tracking
    if (player && player->hasComponent<StateMachine>()) {
        StateMachine* ghostSM = ghost->addComponent<StateMachine>();
        ghostSM->state = static_cast<PlayerState>(first.animState);
        ghostSM->facingRight = first.facingRight;
    }

    // Register ghost with the engine's layer system
    manager.assignObjectToLayer(ghost, 1);

    m_isPlaying = true;
    std::cout << "[Replay] Playback started — " << m_playbackData.frames.size()
              << " frames, ghost spawned.\n";
}

void ReplaySystem::updateGhost(GameObjectManager& manager, float deltaTime) {
    if (!m_isPlaying) return;

    GameObject* ghost = manager.getGameObject(GHOST_OBJECT_NAME);
    if (!ghost) {
        m_isPlaying = false;
        return;
    }

    m_playbackElapsed += deltaTime;

    const auto& frames = m_playbackData.frames;
    const int totalFrames = static_cast<int>(frames.size());

    // Check if playback is past the last frame's timestamp
    if (m_playbackElapsed >= frames.back().timestamp) {
        stopPlayback(manager);
        return;
    }

    // Advance m_playbackFrame so it points to the first frame at or after the current time
    while (m_playbackFrame < totalFrames - 1 &&
           frames[m_playbackFrame + 1].timestamp <= m_playbackElapsed) {
        ++m_playbackFrame;
    }

    // Interpolate between current frame and next frame for smooth playback
    const ReplayFrame& a = frames[m_playbackFrame];
    if (m_playbackFrame + 1 < totalFrames) {
        const ReplayFrame& b = frames[m_playbackFrame + 1];
        float segmentLen = b.timestamp - a.timestamp;
        float t = (segmentLen > 0.f) ? (m_playbackElapsed - a.timestamp) / segmentLen : 0.f;
        t = std::clamp(t, 0.f, 1.f);

        Transform* tr = ghost->getComponent<Transform>();
        if (tr) {
            tr->x = a.x + (b.x - a.x) * t;
            tr->y = a.y + (b.y - a.y) * t;
            tr->scaleX = a.scaleX + (b.scaleX - a.scaleX) * t;
            tr->scaleY = a.scaleY + (b.scaleY - a.scaleY) * t;
            tr->rotation = a.rotation + (b.rotation - a.rotation) * t;
            // Discrete values snap at halfway
            tr->flipX = (t < 0.5f) ? a.flipX : b.flipX;
        }

        StateMachine* sm = ghost->getComponent<StateMachine>();
        if (sm) {
            sm->state = (t < 0.5f) ? static_cast<PlayerState>(a.animState)
                                    : static_cast<PlayerState>(b.animState);
            sm->facingRight = (t < 0.5f) ? a.facingRight : b.facingRight;
        }
    }
    else {
        // Last frame — just snap
        Transform* tr = ghost->getComponent<Transform>();
        if (tr) {
            tr->x = a.x;
            tr->y = a.y;
            tr->scaleX = a.scaleX;
            tr->scaleY = a.scaleY;
            tr->rotation = a.rotation;
            tr->flipX = a.flipX;
        }
        StateMachine* sm = ghost->getComponent<StateMachine>();
        if (sm) {
            sm->state = static_cast<PlayerState>(a.animState);
            sm->facingRight = a.facingRight;
        }
    }
}

void ReplaySystem::stopPlayback(GameObjectManager& manager) {
    if (m_isPlaying) {
        // Destroy the ghost entity
        GameObject* ghost = manager.getGameObject(GHOST_OBJECT_NAME);
        if (ghost) {
            manager.deleteGameObject(ghost);
        }
        m_isPlaying = false;
        m_playbackFrame = 0;
        m_playbackElapsed = 0.f;
        std::cout << "[Replay] Playback stopped, ghost removed.\n";
    }
}

// ============================================================================
// Viewer mode (full input replay, no ghost)
// ============================================================================

void ReplaySystem::startViewerPlayback(const std::string& levelName, const std::string& runId) {
    std::string shortName = extractLevelName(levelName);

    // Determine which run to load from
    std::string useRunId = runId;
    if (useRunId.empty()) {
        // Default to the best (fastest) run
        useRunId = findBestRunId();
    }
    m_viewerRunId = useRunId;

    if (useRunId.empty() || !loadReplay(shortName, m_playbackData, useRunId)) {
        std::cout << "[Replay] No replay found for " << shortName << " — viewer skipped.\n";
        m_viewerFinished = true;
        return;
    }

    if (m_playbackData.frames.empty()) {
        std::cout << "[Replay] Replay has no frames — viewer skipped.\n";
        m_viewerFinished = true;
        return;
    }

    m_playbackFrame = 0;
    m_playbackElapsed = 0.f;
    m_isViewerMode = true;
    m_viewerFinished = false;

    std::cout << "[Replay] Viewer started — " << m_playbackData.frames.size()
              << " frames for " << shortName << ".\n";
}

void ReplaySystem::updateViewer(float deltaTime) {
    if (!m_isViewerMode || m_viewerFinished || m_viewerPaused) return;

    m_playbackElapsed += deltaTime * m_viewerSpeed;

    const auto& frames = m_playbackData.frames;
    const int totalFrames = static_cast<int>(frames.size());

    if (m_playbackElapsed >= frames.back().timestamp) {
        m_viewerFinished = true;
        std::cout << "[Replay] Viewer playback finished.\n";
        return;
    }

    // Advance frame index to match elapsed time
    while (m_playbackFrame < totalFrames - 1 &&
           frames[m_playbackFrame + 1].timestamp <= m_playbackElapsed) {
        ++m_playbackFrame;
    }
    // If seeking backward, rewind frame index
    while (m_playbackFrame > 0 &&
           frames[m_playbackFrame].timestamp > m_playbackElapsed) {
        --m_playbackFrame;
    }
}

void ReplaySystem::stopViewer() {
    m_isViewerMode = false;
    m_viewerFinished = false;
    m_viewerPaused = false;
    m_viewerSpeed = 1.0f;
    m_playbackFrame = 0;
    m_playbackElapsed = 0.f;
    std::cout << "[Replay] Viewer stopped.\n";
}

void ReplaySystem::toggleViewerPause() {
    if (!m_isViewerMode) return;
    m_viewerPaused = !m_viewerPaused;
    std::cout << "[Replay] Viewer " << (m_viewerPaused ? "paused" : "resumed")
              << " at " << m_playbackElapsed << "s\n";
}

void ReplaySystem::seekViewer(float deltaSeconds) {
    if (!m_isViewerMode) return;
    const auto& frames = m_playbackData.frames;
    if (frames.empty()) return;

    m_playbackElapsed += deltaSeconds;
    if (m_playbackElapsed < 0.f) m_playbackElapsed = 0.f;

    float maxTime = frames.back().timestamp;
    if (m_playbackElapsed > maxTime) {
        m_playbackElapsed = maxTime;
        m_viewerFinished = true;
        return;
    }
    m_viewerFinished = false;

    // Rewind or advance frame index to match new elapsed time
    const int totalFrames = static_cast<int>(frames.size());
    while (m_playbackFrame > 0 &&
           frames[m_playbackFrame].timestamp > m_playbackElapsed) {
        --m_playbackFrame;
    }
    while (m_playbackFrame < totalFrames - 1 &&
           frames[m_playbackFrame + 1].timestamp <= m_playbackElapsed) {
        ++m_playbackFrame;
    }
}

void ReplaySystem::stepViewerFrame(int delta) {
    if (!m_isViewerMode) return;
    const auto& frames = m_playbackData.frames;
    if (frames.empty()) return;

    int newFrame = m_playbackFrame + delta;
    if (newFrame < 0) newFrame = 0;
    if (newFrame >= static_cast<int>(frames.size()))
        newFrame = static_cast<int>(frames.size()) - 1;

    m_playbackFrame = newFrame;
    m_playbackElapsed = frames[m_playbackFrame].timestamp;
    m_viewerFinished = false;
}

void ReplaySystem::setViewerSpeed(float speed) {
    m_viewerSpeed = speed;
    std::cout << "[Replay] Viewer speed set to " << speed << "x\n";
}

float ReplaySystem::getViewerDuration() const {
    if (m_playbackData.frames.empty()) return 0.f;
    return m_playbackData.frames.back().timestamp;
}

// Helper to get the current viewer frame safely
static const ReplayFrame s_emptyFrame{};

const ReplayFrame& ReplaySystem_currentFrame(const ReplaySystem& r, const ReplayData& data, int frame) {
    if (frame >= 0 && frame < static_cast<int>(data.frames.size()))
        return data.frames[frame];
    return s_emptyFrame;
}

bool ReplaySystem::getReplayKeyA() const {
    if (!m_isViewerMode || m_viewerPaused || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return false;
    return m_playbackData.frames[m_playbackFrame].inputKeyA;
}
bool ReplaySystem::getReplayKeyD() const {
    if (!m_isViewerMode || m_viewerPaused || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return false;
    return m_playbackData.frames[m_playbackFrame].inputKeyD;
}
bool ReplaySystem::getReplaySpaceTriggered() const {
    if (!m_isViewerMode || m_viewerPaused || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return false;
    return m_playbackData.frames[m_playbackFrame].inputSpaceTriggered;
}
bool ReplaySystem::getReplaySpaceReleased() const {
    if (!m_isViewerMode || m_viewerPaused || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return false;
    return m_playbackData.frames[m_playbackFrame].inputSpaceReleased;
}
bool ReplaySystem::getReplayMouseLeftHeld() const {
    if (!m_isViewerMode || m_viewerPaused || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return false;
    return m_playbackData.frames[m_playbackFrame].inputMouseLeftHeld;
}
float ReplaySystem::getReplayMouseX() const {
    if (!m_isViewerMode || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return 0.f;
    return m_playbackData.frames[m_playbackFrame].inputMouseX;
}
float ReplaySystem::getReplayMouseY() const {
    if (!m_isViewerMode || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return 0.f;
    return m_playbackData.frames[m_playbackFrame].inputMouseY;
}
float ReplaySystem::getReplayX() const {
    if (!m_isViewerMode || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return 0.f;
    return m_playbackData.frames[m_playbackFrame].x;
}
float ReplaySystem::getReplayY() const {
    if (!m_isViewerMode || m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return 0.f;
    return m_playbackData.frames[m_playbackFrame].y;
}

// ============================================================================
// World snapshot application (viewer mode)
// ============================================================================

void ReplaySystem::applyWorldSnapshot(GameObjectManager& manager) {
    if (!m_isViewerMode) return;
    if (m_playbackFrame < 0 || m_playbackFrame >= static_cast<int>(m_playbackData.frames.size())) return;

    const ReplayFrame& frame = m_playbackData.frames[m_playbackFrame];
    for (const auto& snap : frame.worldObjects) {
        GameObject* obj = manager.getGameObject(snap.objectName);
        if (!obj) continue;

        if (StateMachine* sm = obj->getComponent<StateMachine>())
            sm->state = static_cast<PlayerState>(snap.smState);
        if (CollisionInfo* col = obj->getComponent<CollisionInfo>()) {
            col->collisionTag = snap.collisionTag;
            col->collisionFlag = snap.collisionFlag;
        }
        if (Render* r = obj->getComponent<Render>())
            r->clr.a = snap.alpha;
        if (Transform* t = obj->getComponent<Transform>()) {
            t->x = snap.posX;
            t->y = snap.posY;
        }
    }
}

// ============================================================================
// File I/O — RapidJSON
// ============================================================================

bool ReplaySystem::saveReplay(const ReplayData& data) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& a = doc.GetAllocator();

    doc.AddMember("levelName", rapidjson::Value(data.levelName.c_str(), a), a);
    doc.AddMember("completionTimeMs", data.completionTimeMs, a);
    doc.AddMember("frameCount", static_cast<int>(data.frames.size()), a);

    rapidjson::Value framesArr(rapidjson::kArrayType);
    for (const auto& f : data.frames) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("t", f.timestamp, a);
        obj.AddMember("x", f.x, a);
        obj.AddMember("y", f.y, a);
        obj.AddMember("sx", f.scaleX, a);
        obj.AddMember("sy", f.scaleY, a);
        obj.AddMember("r", f.rotation, a);
        obj.AddMember("fl", f.flipX ? 1 : 0, a);
        obj.AddMember("st", f.animState, a);
        obj.AddMember("fr", f.facingRight ? 1 : 0, a);
        // Input state for replay viewer
        obj.AddMember("kA", f.inputKeyA ? 1 : 0, a);
        obj.AddMember("kD", f.inputKeyD ? 1 : 0, a);
        obj.AddMember("kST", f.inputSpaceTriggered ? 1 : 0, a);
        obj.AddMember("kSR", f.inputSpaceReleased ? 1 : 0, a);
        obj.AddMember("mL", f.inputMouseLeftHeld ? 1 : 0, a);
        obj.AddMember("mX", f.inputMouseX, a);
        obj.AddMember("mY", f.inputMouseY, a);
        // World object snapshots
        if (!f.worldObjects.empty()) {
            rapidjson::Value woArr(rapidjson::kArrayType);
            for (const auto& wo : f.worldObjects) {
                rapidjson::Value woObj(rapidjson::kObjectType);
                woObj.AddMember("n", rapidjson::Value(wo.objectName.c_str(), a), a);
                woObj.AddMember("sm", wo.smState, a);
                woObj.AddMember("ct", rapidjson::Value(wo.collisionTag.c_str(), a), a);
                woObj.AddMember("cf", wo.collisionFlag ? 1 : 0, a);
                woObj.AddMember("al", wo.alpha, a);
                woObj.AddMember("px", wo.posX, a);
                woObj.AddMember("py", wo.posY, a);
                woArr.PushBack(woObj, a);
            }
            obj.AddMember("wo", woArr, a);
        }
        framesArr.PushBack(obj, a);
    }
    doc.AddMember("frames", framesArr, a);

    const std::string path = getReplayPath(data.levelName, "current");
    // Ensure current/ dir exists
    std::filesystem::create_directories(getRunDir("current"));
    std::string err;
    if (!JsonIO::WriteDocumentToFile(path, doc, false, &err)) {
        std::cerr << "[Replay] Failed to save replay: " << err << "\n";
        return false;
    }

    std::cout << "[Replay] Saved " << data.frames.size() << " frames to " << path << "\n";
    return true;
}

bool ReplaySystem::loadReplay(const std::string& levelName, ReplayData& outData,
                              const std::string& runId) const {
    const std::string path = getReplayPath(levelName, runId);

    if (!std::filesystem::exists(path)) return false;

    rapidjson::Document doc;
    std::string err;
    if (!JsonIO::ReadFileToDocument(path, doc, &err)) {
        std::cerr << "[Replay] Failed to load " << path << ": " << err << "\n";
        return false;
    }

    outData = ReplayData{};

    if (doc.HasMember("levelName") && doc["levelName"].IsString())
        outData.levelName = doc["levelName"].GetString();
    if (doc.HasMember("completionTimeMs") && doc["completionTimeMs"].IsInt())
        outData.completionTimeMs = doc["completionTimeMs"].GetInt();

    if (doc.HasMember("frames") && doc["frames"].IsArray()) {
        const auto& arr = doc["frames"];
        outData.frames.reserve(arr.Size());

        for (rapidjson::SizeType i = 0; i < arr.Size(); ++i) {
            const auto& obj = arr[i];
            ReplayFrame f;

            if (obj.HasMember("t"))  f.timestamp = obj["t"].GetFloat();
            if (obj.HasMember("x"))  f.x  = obj["x"].GetFloat();
            if (obj.HasMember("y"))  f.y  = obj["y"].GetFloat();
            if (obj.HasMember("sx")) f.scaleX = obj["sx"].GetFloat();
            if (obj.HasMember("sy")) f.scaleY = obj["sy"].GetFloat();
            if (obj.HasMember("r"))  f.rotation = obj["r"].GetFloat();
            if (obj.HasMember("fl")) f.flipX = obj["fl"].GetInt() != 0;
            if (obj.HasMember("st")) f.animState = obj["st"].GetInt();
            if (obj.HasMember("fr")) f.facingRight = obj["fr"].GetInt() != 0;
            if (obj.HasMember("kA"))  f.inputKeyA = obj["kA"].GetInt() != 0;
            if (obj.HasMember("kD"))  f.inputKeyD = obj["kD"].GetInt() != 0;
            if (obj.HasMember("kST")) f.inputSpaceTriggered = obj["kST"].GetInt() != 0;
            if (obj.HasMember("kSR")) f.inputSpaceReleased = obj["kSR"].GetInt() != 0;
            if (obj.HasMember("mL"))  f.inputMouseLeftHeld = obj["mL"].GetInt() != 0;
            if (obj.HasMember("mX"))  f.inputMouseX = obj["mX"].GetFloat();
            if (obj.HasMember("mY"))  f.inputMouseY = obj["mY"].GetFloat();

            // World object snapshots (backward compatible — absent in old files)
            if (obj.HasMember("wo") && obj["wo"].IsArray()) {
                for (const auto& woVal : obj["wo"].GetArray()) {
                    WorldObjectSnapshot snap;
                    if (woVal.HasMember("n") && woVal["n"].IsString())
                        snap.objectName = woVal["n"].GetString();
                    if (woVal.HasMember("sm"))  snap.smState = woVal["sm"].GetInt();
                    if (woVal.HasMember("ct") && woVal["ct"].IsString())
                        snap.collisionTag = woVal["ct"].GetString();
                    if (woVal.HasMember("cf"))  snap.collisionFlag = woVal["cf"].GetInt() != 0;
                    if (woVal.HasMember("al"))  snap.alpha = woVal["al"].GetFloat();
                    if (woVal.HasMember("px"))  snap.posX = woVal["px"].GetFloat();
                    if (woVal.HasMember("py"))  snap.posY = woVal["py"].GetFloat();
                    f.worldObjects.push_back(snap);
                }
            }

            outData.frames.push_back(f);
        }
    }

    std::cout << "[Replay] Loaded replay for " << outData.levelName
              << " from run " << runId << " — " << outData.frames.size() << " frames\n";
    return true;
}

bool ReplaySystem::hasReplay(const std::string& levelName) const {
    std::string shortName = extractLevelName(levelName);
    // Check any completed run (not current/)
    std::string bestRun = findBestRunId();
    if (!bestRun.empty()) {
        return std::filesystem::exists(getReplayPath(shortName, bestRun));
    }
    return false;
}

bool ReplaySystem::hasReplayForRun(const std::string& levelName, const std::string& runId) const {
    return std::filesystem::exists(getReplayPath(extractLevelName(levelName), runId));
}

// ============================================================================
// Run management
// ============================================================================

void ReplaySystem::finalizeRun(int totalTimeMs) {
    std::string currentDir = getRunDir("current");
    if (!std::filesystem::exists(currentDir) || std::filesystem::is_empty(currentDir)) {
        std::cout << "[Replay] No current run to finalize.\n";
        return;
    }

    std::string runId = std::to_string(totalTimeMs);
    std::string destDir = getRunDir(runId);

    // If a run with the exact same time exists, remove it first
    if (std::filesystem::exists(destDir)) {
        std::filesystem::remove_all(destDir);
    }

    std::filesystem::rename(currentDir, destDir);
    // Recreate current/ for next run
    std::filesystem::create_directories(currentDir);

    std::cout << "[Replay] Finalized run as " << runId << "ms\n";
}

void ReplaySystem::clearCurrentRun() {
    std::string currentDir = getRunDir("current");
    if (std::filesystem::exists(currentDir)) {
        std::filesystem::remove_all(currentDir);
    }
    std::filesystem::create_directories(currentDir);
    std::cout << "[Replay] Cleared current run.\n";
}

std::string ReplaySystem::findBestRunId() const {
    std::string replayDir = getReplayDir();
    if (!std::filesystem::exists(replayDir)) return "";

    int bestTime = INT_MAX;
    std::string bestId;

    for (const auto& entry : std::filesystem::directory_iterator(replayDir)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        if (name == "current") continue;

        // Folder names are total time in ms (e.g. "45230")
        try {
            int timeMs = std::stoi(name);
            if (timeMs < bestTime) {
                bestTime = timeMs;
                bestId = name;
            }
        } catch (...) {
            continue; // skip non-numeric folder names
        }
    }
    return bestId;
}

std::vector<std::string> ReplaySystem::getAllRunIds() const {
    std::vector<std::string> ids;
    std::string replayDir = getReplayDir();
    if (!std::filesystem::exists(replayDir)) return ids;

    for (const auto& entry : std::filesystem::directory_iterator(replayDir)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        if (name == "current") continue;
        try {
            (void)std::stoi(name); // validate it's numeric
            ids.push_back(name);
        } catch (...) {
            continue;
        }
    }
    // Sort ascending by time
    std::sort(ids.begin(), ids.end(), [](const std::string& a, const std::string& b) {
        return std::stoi(a) < std::stoi(b);
    });
    return ids;
}
