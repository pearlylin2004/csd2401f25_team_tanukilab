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

// ============================================================================
// Helper Functions
// ============================================================================

/*!
 * \brief Checks if any movement key (A/D) is currently held.
 * \return True if A or D is pressed.
 */
static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

/*!
 * \brief Resets animation to the initial frame for a given state.
 * \param obj The GameObject with Animation component.
 * \param state The PlayerState whose animation frame to reset.
 */
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
// IDLE STATE
// ============================================================================

void IdleState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Idle);

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Idle";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void IdleState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // Transitions handle all logic
}

// ============================================================================
// WALKING STATE
// ============================================================================

void WalkingState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Walking);

    // Play footsteps audio
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* footsteps = audio->getOrCreateChannel("footsteps");
        if (footsteps) {
            footsteps->audioFile = "assets/audio/footsteps.wav";
            footsteps->loop = true;
            footsteps->volume = 0.5f;
            footsteps->isPendingPlay = true;
        }
    }

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Walking";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void WalkingState::onUpdate(GameObject& obj, float dt) {
    (void)dt;

    // Update facing direction based on input
    if (StateMachine* fsm = obj.getComponent<StateMachine>()) {
        if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
            fsm->facingRight = false;
        }
        if (InputHandler::isKeyHeld(GLFW_KEY_D)) {
            fsm->facingRight = true;
        }
    }
}

void WalkingState::onExit(GameObject& obj) {
    // Stop footsteps audio with fade out
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* footsteps = audio->getChannel("footsteps");
        if (footsteps && footsteps->state == AudioState::Playing) {
            footsteps->isPendingStop = true;
            footsteps->fadeOutOnStop = true;
            footsteps->fadeOutDuration = 0.15f;
        }
    }
}

// ============================================================================
// JUMPING STATE
// ============================================================================

void JumpingState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Jumping);

    // Play jump sound
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* jump = audio->getOrCreateChannel("jump");
        if (jump) {
            jump->audioFile = "assets/audio/jump.wav";
            jump->loop = false;
            jump->volume = 0.7f;
            jump->isPendingPlay = true;
        }
    }

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Jumping";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void JumpingState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // Transition to Falling handled by condition check
}

// ============================================================================
// FALLING STATE
// ============================================================================

void FallingState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // Landing sound played in transition action
}

// ============================================================================
// DEAD STATE
// ============================================================================

void DeadState::onEnter(GameObject& obj) {
    resetAnimationFrame(obj, PlayerState::Dead);

    // Disable movement
    if (Physics* p = obj.getComponent<Physics>()) {
        p->velX = 0.0f;
        p->velY = 0.0f;
        p->onGround = true;
        p->canMove = false;
    }

    std::string msg = "[FSM] " + obj.getObjectName() + " -> Dead";
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

void DeadState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;

    // Could add respawn timer here
    // Example:
    // if (stateTime > 2.0f) {
    //     // Trigger respawn
    // }
}