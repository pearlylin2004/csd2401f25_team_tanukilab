/* Start Header ************************************************************************/
/*!
\file		LogicContainer.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the LogicContainer class. Handles player logic transitions
            such as Idle, Walking, and Jumping based on input and state.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LogicContainer.h"
#include "GameObject.h"
#include "input.h"
#include "Systems.h"
#include "Component.h"
#include "GameObjectManager.h"

// ============================================================================
// Helper: convert PlayerState enum to readable string
// ============================================================================
/*!
 * \brief Converts a PlayerState enum into a human-readable C-string.
 * \param s The PlayerState to convert.
 * \return A string literal representing the state.
 */
static const char* ToString(PlayerState s)
{
    switch (s)
    {
    case PlayerState::Idle:    return "Idle";
    case PlayerState::Walking: return "Walking";
    case PlayerState::Jumping: return "Jumping";
    case PlayerState::Falling: return "Falling";
    case PlayerState::Shooting:return "Shooting";
    default:                   return "Unknown";
    }
}

/*!
 * \brief Checks if any movement key (W/A/S/D) is currently held.
 * \return True if one or more movement keys are held.
 */
static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_W) ||
        InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_S) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

// ============================================================================
// FSM Transition Helper
// ============================================================================
/*!
 * \brief Applies a state transition to the object’s StateMachine component.
 *
 * Resets state timer, logs the transition to ImGui & console,
 * and performs any enter-state side effects.
 *
 * \param obj Reference to the GameObject owning the FSM.
 * \param next The next PlayerState to enter.
 */
void LogicContainer::enter(GameObject& obj, PlayerState next)
{
    if (auto* fsm = obj.getComponent<StateMachine>())
    {
        if (fsm->state != next)
        {
            // reset animation frame
            if (Animation* a = obj.getComponent<Animation>()) {
                a->animState[static_cast<int>(fsm->state)].currentFrameColumn = a->animState[static_cast<int>(fsm->state)].initialFrame.x;
                a->animState[static_cast<int>(fsm->state)].currentFrameRow = a->animState[static_cast<int>(fsm->state)].initialFrame.y;
            }
            std::string msg = "[LogicContainer] "
                + obj.getObjectName() + " -> " + ToString(next);
            DebugLog::addMessage(msg, DebugMode::PlaySimul);
            std::cout << msg << std::endl;
        }

        fsm->state = next;
        fsm->stateTime = 0.0f;
    }
}

// ============================================================================
// State: Idle
// ============================================================================
/*!
 * \brief Per-frame update for Idle state.
 *
 * Handles:
 * - Jumping (B key)
 * - Transition to Walking when movement keys are held
 *
 * \param obj Player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickIdle(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    // jump
    if (InputHandler::isKeyTriggered(GLFW_KEY_B) && physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        enter(obj, PlayerState::Jumping);
        DebugLog::addMessage("FSM: Jump", DebugMode::PlaySimul);
    }
    // walk
    else if (anyMoveHeld()) {
        enter(obj, PlayerState::Walking);
    }
}

// ============================================================================
// State: Walking
// ============================================================================
/*!
 * \brief Per-frame update for Walking state.
 *
 * Handles:
 * - Facing direction based on A/D keys
 * - Returning to Idle when no movement keys are held
 * - Transition to Falling when leaving ground
 * - Jumping (B key)
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickWalking(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    auto* fsm = obj.getComponent<StateMachine>();
    if (!physics || !fsm) return;

    if (InputHandler::isKeyHeld(GLFW_KEY_A)) fsm->facingRight = false;
    if (InputHandler::isKeyHeld(GLFW_KEY_D)) fsm->facingRight = true;

    if (!anyMoveHeld()) {
        enter(obj, PlayerState::Idle);
        return;
    }
    if (!physics->onGround && physics->velY < 0.0f) {
        enter(obj, PlayerState::Falling);
        return;
    }
    if (InputHandler::isKeyTriggered(GLFW_KEY_B) && physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        enter(obj, PlayerState::Jumping);
        DebugLog::addMessage("FSM: Jump", DebugMode::PlaySimul);
        return;
    }
}

// ============================================================================
// State: Jumping
// ============================================================================
/*!
 * \brief Per-frame update for Jumping state.
 *
 * Switches to Falling once vertical velocity becomes negative.
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickJumping(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->velY <= 0.0f) {
        enter(obj, PlayerState::Falling);
    }
}

// ============================================================================
// State: Falling
// ============================================================================
/*!
 * \brief Per-frame update for Falling state.
 *
 * When the player lands:
 * - enters Walking if movement keys held
 * - otherwise enters Idle
 *
 * \param obj The player GameObject.
 * \param dt Delta time (unused).
 */
void LogicContainer::tickFalling(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->onGround) {
        if (anyMoveHeld()) enter(obj, PlayerState::Walking);
        else               enter(obj, PlayerState::Idle);
    }
}

// ============================================================================
// Shooting Logic
// ============================================================================
/*!
 * \brief Spawns a bullet near the player and gives it initial velocity.
 *
 * Uses the player's Transform and facing direction to position the bullet.
 *
 * \param obj The player.
 * \param manager Reference to GameObjectManager for accessing existing bullet object.
 */
//void LogicContainer::trySpawnBulletNear(GameObject& obj, GameObjectManager& manager) {
//    auto* playerT = obj.getComponent<Transform>();
//    auto* fsm = obj.getComponent<StateMachine>();
//    if (!playerT || !fsm) return;
//
//    GameObject* bullet = manager.getGameObject("bullet");
//    if (!bullet) return;
//
//    Transform* t = bullet->getComponent<Transform>();
//    Physics* p = bullet->getComponent<Physics>();
//    if (!t || !p) return;
//
//    const bool right = fsm->facingRight;
//    t->x = playerT->x + (right ? 0.8f : -0.8f);
//    t->y = playerT->y + 0.3f;
//    t->z = 0.02f;
//    t->scaleX = t->scaleY = t->scaleZ = 0.5f;
//
//    p->velX = right ? 6.0f : -6.0f;
//    p->velY = 5.0f;
//    p->onGround = false;
//
//    DebugLog::addMessage("[LogicContainer] player fired bullet", DebugMode::PlaySimul);
//    std::cout << "[LogicContainer] player fired bullet\n";
//}

// ============================================================================
// Main Update
// ============================================================================
/*!
 * \brief Main per-frame update for the LogicContainer.
 *
 * Responsibilities:
 * - Increase state time
 * - Handle shooting (SPACE key)
 * - Run per-state tick function (Idle/Walking/Jumping/Falling)
 *
 * \param obj The player GameObject being updated.
 * \param dt Delta time in seconds.
 * \param manager GameObjectManager used for spawning bullets.
 */
void LogicContainer::update(GameObject& obj, float dt, GameObjectManager& manager) {
    auto* fsm = obj.getComponent<StateMachine>();
    if (!fsm) return;

    fsm->stateTime += dt;

    // Immediate action: shooting
    /*if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE)) {
        trySpawnBulletNear(obj, manager);
    }*/

    switch (fsm->state) {
    case PlayerState::Idle:     tickIdle(obj, dt);     break;
    case PlayerState::Walking:  tickWalking(obj, dt);  break;
    case PlayerState::Jumping:  tickJumping(obj, dt);  break;
    case PlayerState::Falling:  tickFalling(obj, dt);  break;
    case PlayerState::Dead:                            break;
    default: break;
    }
}