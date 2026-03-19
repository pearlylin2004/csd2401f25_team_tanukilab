/* Start Header ************************************************************************/
/*!
\file		PlayerStates.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements class-based player states for the FSM.
            Handles enter/update/exit logic for Idle, Walking, Jumping, Falling, Dead.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PlayerStates.h"
#include "GameObject.h"
#include "Component.h"
#include "input.h"
#include "Systems.h"
#include <iostream>
#include <random>

// ============================================================================
// Helper Functions
// ============================================================================

static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

static float randf(float min, float max) {
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

static int randInt(int min, int maxExclusive) {
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(min, maxExclusive - 1);
    return dist(gen);
}

static void resetAnimationFrame(GameObject& obj, PlayerState state) {
    if (Animation* a = obj.getComponent<Animation>()) {
        int idx = static_cast<int>(state);
        if (idx >= 0 && idx < static_cast<int>(a->animState.size())) {
            a->animState[idx].currentFrameColumn =
                static_cast<int>(a->animState[idx].initialFrame.x);
            a->animState[idx].currentFrameRow =
                static_cast<int>(a->animState[idx].initialFrame.y);
        }
    }
}

// ============================================================================
// Footstep Sound Pools
// ============================================================================

// Add your footstep .wav files here (at least 3 for good variety)
static const std::vector<std::string> s_footstepFiles = {
    //"assets/audio/Foot Step 1.wav",
    //"assets/audio/Foot Step 2.wav",
    //"assets/audio/Foot Step 3.wav",
    //"assets/audio/Foot Step 4.wav",
    //"assets/audio/Foot Step 5.wav"

    "assets/audio/Footsteps2_Gravel-Dirt_far-small_1.wav",
    "assets/audio/Footsteps2_Gravel-Dirt_far-small_2.wav",
    "assets/audio/Footsteps2_Gravel-Dirt_far-small_3.wav",
    "assets/audio/Footsteps2_Gravel-Dirt_far-small_4.wav",
    "assets/audio/Footsteps2_Gravel-Dirt_far-small_5.wav",
    "assets/audio/Footsteps2_Gravel-Dirt_far-small_6.wav"
};

// Landing sound pool (played on Falling -> Idle/Walking transitions)
static const std::vector<std::string> s_landingFiles = {
    "assets/audio/landing.wav",
    
};

// Which animation frames trigger a footstep sound.
// These are the frames where the foot hits the ground in your walk spritesheet.
// *** ADJUST THESE to match your walking animation ***
static const std::vector<int> s_footContactFrames = { 1, 4 };

// ============================================================================
// IDLE STATE
// ============================================================================

void IdleState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Idle);
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        if (AudioChannel* footstep = audio->getChannel("footstep")) {
            if (footstep->state == AudioState::Playing) {
                footstep->isPendingStop = true;
            }
        }
    }
    std::string msg = "[FSM] " + obj.getObjectName() + " -> Idle";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void IdleState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
}

// ============================================================================
// WALKING STATE
// ============================================================================

void WalkingState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Walking);
    m_lastFrame = -1;
    m_lastSoundIdx = -1;

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Walking";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void WalkingState::onUpdate(GameObject& obj, float dt) {
    (void)dt;

    // Update facing direction
    if (StateMachine* fsm = obj.getComponent<StateMachine>()) {
        if (InputHandler::isKeyHeld(GLFW_KEY_A)) fsm->facingRight = false;
        if (InputHandler::isKeyHeld(GLFW_KEY_D)) fsm->facingRight = true;
    }

    // Sync footsteps to animation frames
    if (Animation* anim = obj.getComponent<Animation>()) {
        int walkIdx = static_cast<int>(PlayerState::Walking);
        if (walkIdx < static_cast<int>(anim->animState.size())) {
            int curFrame = anim->animState[walkIdx].currentFrameColumn;

            // Only act when the frame actually changes
            if (curFrame != m_lastFrame) {
                m_lastFrame = curFrame;

                // Check if this is a foot-contact frame
                for (int contactFrame : s_footContactFrames) {
                    if (curFrame == contactFrame) {
                        playRandomFootstep(obj);
                        break;
                    }
                }
            }
        }
    }
}

void WalkingState::playRandomFootstep(GameObject& obj) {
    AudioComponent* audio = obj.getComponent<AudioComponent>();
    if (!audio || s_footstepFiles.empty()) return;

    AudioChannel* step = audio->getOrCreateChannel("footstep");
    if (!step) return;

    // Pick a random index that differs from the last one played
    int idx = randInt(0, static_cast<int>(s_footstepFiles.size()));
    if (s_footstepFiles.size() > 1) {
        while (idx == m_lastSoundIdx) {
            idx = randInt(0, static_cast<int>(s_footstepFiles.size()));
        }
    }
    m_lastSoundIdx = idx;

    step->audioFile = s_footstepFiles[idx];
    step->loop = false;
    step->pitch = randf(0.90f, 1.10f);
    step->volume = randf(0.30f, 0.45f);
    step->playOnStart = false;
    step->isPendingPlay = true;
}

void WalkingState::onExit(GameObject& obj) {
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        if (AudioChannel* step = audio->getChannel("footstep")) {
            if (step->state == AudioState::Playing) {
                step->isPendingStop = true;
            }
        }
    }
}

// ============================================================================
// JUMPING STATE
// ============================================================================

void JumpingState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Jumping);

    /*if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* jump = audio->getOrCreateChannel("jump");
        if (jump) {
            jump->audioFile = "assets/audio/jump.wav";
            jump->loop = false;
            jump->volume = 0.7f;
            jump->isPendingPlay = true;
        }
    }*/

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Jumping";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void JumpingState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
}

// ============================================================================
// FALLING STATE
// ============================================================================

void FallingState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
}

// ============================================================================
// DEAD STATE
// ============================================================================

void DeadState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Dead);

    if (Physics* p = obj.getComponent<Physics>()) {
        p->velX = 0.0f;
        p->velY = 0.0f;
        p->gravity = 0.0f;
        p->canMove = false;
        p->physicsFlag = false;
    }

    if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
        c->collisionFlag = false;
    }

    if (Render* r = obj.getComponent<Render>()) {
        r->texFile = "assets/textures/smoke.png";
        r->texChanged = true;
        r->hasTex = true;
    }

    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* death = audio->getOrCreateChannel("death");
        if (death) {
            death->audioFile = "assets/audio/death_poof.wav";
            death->loop = false;
            death->volume = 0.8f;
            death->isPendingPlay = true;
        }
    }

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Dead (transformed to smoke)";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void DeadState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
}