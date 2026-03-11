/* Start Header ************************************************************************/
/*!
\file       ReplaySystem.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       March, 11th, 2026
\brief      Singleton replay and ghost system. Records per-frame player state
            during gameplay, saves the best run per level to disk, and can
            play it back through a semi-transparent ghost entity.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <cstdint>

class GameObjectManager;

// Per-frame snapshot of the player's visible state
struct ReplayFrame {
    float timestamp = 0.f; // seconds elapsed since recording started
    float x = 0.f;
    float y = 0.f;
    float scaleX = 1.f;
    float scaleY = 1.f;
    float rotation = 0.f;
    bool  flipX = false;
    int   animState = 0;   // PlayerState enum cast to int
    bool  facingRight = true;

    // Recorded input state for full replay viewer
    bool  inputKeyA = false;
    bool  inputKeyD = false;
    bool  inputSpaceTriggered = false;
    bool  inputSpaceReleased = false;
    bool  inputMouseLeftHeld = false;
    float inputMouseX = 0.f;
    float inputMouseY = 0.f;
};

// Full recording of a single run
struct ReplayData {
    std::string levelName;
    int completionTimeMs = 0;
    std::vector<ReplayFrame> frames;
};

class ReplaySystem {
public:
    static ReplaySystem& getInstance() {
        static ReplaySystem instance;
        return instance;
    }

    void init();

    // --- Recording ---
    void startRecording(const std::string& levelName);
    void recordFrame(GameObjectManager& manager, float deltaTime);
    void stopRecording(int completionTimeMs);
    void discardRecording();

    // --- Playback & Ghost ---
    // Loads the best replay for the given level, spawns the ghost entity
    void startPlayback(const std::string& levelName, GameObjectManager& manager);
    // Advances the ghost using elapsed time; call once per update tick
    void updateGhost(GameObjectManager& manager, float deltaTime);
    // Stops playback and destroys the ghost entity
    void stopPlayback(GameObjectManager& manager);

    // --- Viewer mode (full input replay, no ghost) ---
    void startViewerPlayback(const std::string& levelName);
    void updateViewer(float deltaTime);
    void stopViewer();
    bool isViewerMode() const { return m_isViewerMode; }
    bool isViewerFinished() const { return m_viewerFinished; }

    // Query recorded inputs during viewer playback
    bool getReplayKeyA() const;
    bool getReplayKeyD() const;
    bool getReplaySpaceTriggered() const;
    bool getReplaySpaceReleased() const;
    bool getReplayMouseLeftHeld() const;
    float getReplayMouseX() const;
    float getReplayMouseY() const;
    // Position from recording for periodic correction
    float getReplayX() const;
    float getReplayY() const;

    // --- State queries ---
    bool isRecording() const { return m_isRecording; }
    bool isPlaying()   const { return m_isPlaying; }

    // --- File I/O ---
    bool saveReplay(const ReplayData& data) const;
    bool loadReplay(const std::string& levelName, ReplayData& outData) const;
    bool hasReplay(const std::string& levelName) const;

    static constexpr const char* GHOST_OBJECT_NAME = "__ghost__";

private:
    ReplaySystem() = default;
    ~ReplaySystem() = default;
    ReplaySystem(const ReplaySystem&) = delete;
    ReplaySystem& operator=(const ReplaySystem&) = delete;

    std::string getReplayDir() const;
    std::string getReplayPath(const std::string& levelName) const;
    // Extract short level name from full path (e.g. "level01" from "./assets/Scene/level01.json")
    static std::string extractLevelName(const std::string& fullPath);

    bool        m_isRecording = false;
    bool        m_isPlaying = false;
    ReplayData  m_currentRecording;
    ReplayData  m_playbackData;
    int         m_playbackFrame = 0;
    float       m_recordingElapsed = 0.f;  // seconds elapsed during recording
    float       m_playbackElapsed = 0.f;   // seconds elapsed during playback
    bool        m_initialized = false;
    bool        m_isViewerMode = false;
    bool        m_viewerFinished = false;
};
