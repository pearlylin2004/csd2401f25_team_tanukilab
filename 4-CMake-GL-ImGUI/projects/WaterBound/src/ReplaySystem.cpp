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

std::string ReplaySystem::getReplayPath(const std::string& levelName) const {
    return getReplayDir() + "/" + levelName + "_best.json";
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
    // Ensure replays directory exists
    std::filesystem::create_directories(getReplayDir());
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
    frame.inputKeyA = InputHandler::isKeyHeld(GLFW_KEY_A);
    frame.inputKeyD = InputHandler::isKeyHeld(GLFW_KEY_D);
    frame.inputSpaceTriggered = InputHandler::isKeyTriggered(GLFW_KEY_SPACE);
    frame.inputSpaceReleased = InputHandler::isKeyReleased(GLFW_KEY_SPACE);
    frame.inputMouseLeftHeld = InputHandler::isMouseLeftHeld();
    Vector2D mousePos = InputHandler::getWorldMousePosition();
    frame.inputMouseX = static_cast<float>(mousePos.x);
    frame.inputMouseY = static_cast<float>(mousePos.y);

    m_currentRecording.frames.push_back(frame);
}

void ReplaySystem::stopRecording(int completionTimeMs) {
    if (!m_isRecording) return;

    m_isRecording = false;
    m_currentRecording.completionTimeMs = completionTimeMs;

    std::cout << "[Replay] Recording stopped — " << m_currentRecording.frames.size()
              << " frames, " << completionTimeMs << "ms\n";

    // Only save if this is the best (fastest) run for this level
    ReplayData existing;
    bool hasPrevious = loadReplay(m_currentRecording.levelName, existing);

    if (!hasPrevious || completionTimeMs < existing.completionTimeMs) {
        saveReplay(m_currentRecording);
        std::cout << "[Replay] New best time! Saved replay.\n";
    }
    else {
        std::cout << "[Replay] Not a new best time (" << completionTimeMs
                  << "ms vs " << existing.completionTimeMs << "ms). Replay not saved.\n";
    }
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

    if (!loadReplay(shortName, m_playbackData)) {
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

void ReplaySystem::startViewerPlayback(const std::string& levelName) {
    std::string shortName = extractLevelName(levelName);

    if (!loadReplay(shortName, m_playbackData)) {
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
    if (!m_isViewerMode || m_viewerFinished) return;

    m_playbackElapsed += deltaTime;

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
}

void ReplaySystem::stopViewer() {
    m_isViewerMode = false;
    m_viewerFinished = false;
    m_playbackFrame = 0;
    m_playbackElapsed = 0.f;
    std::cout << "[Replay] Viewer stopped.\n";
}

// Helper to get the current viewer frame safely
static const ReplayFrame s_emptyFrame{};

const ReplayFrame& ReplaySystem_currentFrame(const ReplaySystem& r, const ReplayData& data, int frame) {
    if (frame >= 0 && frame < static_cast<int>(data.frames.size()))
        return data.frames[frame];
    return s_emptyFrame;
}

bool ReplaySystem::getReplayKeyA() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputKeyA : false;
}
bool ReplaySystem::getReplayKeyD() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputKeyD : false;
}
bool ReplaySystem::getReplaySpaceTriggered() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputSpaceTriggered : false;
}
bool ReplaySystem::getReplaySpaceReleased() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputSpaceReleased : false;
}
bool ReplaySystem::getReplayMouseLeftHeld() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputMouseLeftHeld : false;
}
float ReplaySystem::getReplayMouseX() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputMouseX : 0.f;
}
float ReplaySystem::getReplayMouseY() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].inputMouseY : 0.f;
}
float ReplaySystem::getReplayX() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].x : 0.f;
}
float ReplaySystem::getReplayY() const {
    return m_isViewerMode ? m_playbackData.frames[m_playbackFrame].y : 0.f;
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
        framesArr.PushBack(obj, a);
    }
    doc.AddMember("frames", framesArr, a);

    const std::string path = getReplayPath(data.levelName);
    std::string err;
    if (!JsonIO::WriteDocumentToFile(path, doc, false, &err)) {
        std::cerr << "[Replay] Failed to save replay: " << err << "\n";
        return false;
    }

    std::cout << "[Replay] Saved " << data.frames.size() << " frames to " << path << "\n";
    return true;
}

bool ReplaySystem::loadReplay(const std::string& levelName, ReplayData& outData) const {
    const std::string path = getReplayPath(levelName);

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
            // Input state (backward-compatible: defaults to false/0 if absent)
            if (obj.HasMember("kA"))  f.inputKeyA = obj["kA"].GetInt() != 0;
            if (obj.HasMember("kD"))  f.inputKeyD = obj["kD"].GetInt() != 0;
            if (obj.HasMember("kST")) f.inputSpaceTriggered = obj["kST"].GetInt() != 0;
            if (obj.HasMember("kSR")) f.inputSpaceReleased = obj["kSR"].GetInt() != 0;
            if (obj.HasMember("mL"))  f.inputMouseLeftHeld = obj["mL"].GetInt() != 0;
            if (obj.HasMember("mX"))  f.inputMouseX = obj["mX"].GetFloat();
            if (obj.HasMember("mY"))  f.inputMouseY = obj["mY"].GetFloat();

            outData.frames.push_back(f);
        }
    }

    std::cout << "[Replay] Loaded replay for " << outData.levelName
              << " — " << outData.frames.size() << " frames\n";
    return true;
}

bool ReplaySystem::hasReplay(const std::string& levelName) const {
    return std::filesystem::exists(getReplayPath(extractLevelName(levelName)));
}
