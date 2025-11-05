/* Start Header ************************************************************************/
/*!
\file		LogicContainer.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 30th, 2025
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
#include "ui.h"   


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


static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_W) ||
        InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_S) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

// --- transitions ---
void LogicContainer::enter(GameObject& obj, PlayerState next)
{
    if (auto* fsm = obj.getComponent<StateMachine>())
    {
        if (fsm->state != next)
        {
            std::string msg = "[LogicContainer] "
                + obj.getObjectName() + " -> " + ToString(next);
            UI::addDebugMessage(msg);          // shows in ImGui debug console
            std::cout << msg << std::endl;     // shows in terminal
        }

        fsm->state = next;
        fsm->stateTime = 0.0f;
    }
}


void LogicContainer::tickIdle(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (InputHandler::isKeyTriggered(GLFW_KEY_B) && physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        enter(obj, PlayerState::Jumping);
        UI::addDebugMessage("FSM: Jump");
    }
    else if (anyMoveHeld()) {
        enter(obj, PlayerState::Walking);
    }
}

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
        UI::addDebugMessage("FSM: Jump");
        return;
    }
}

void LogicContainer::tickJumping(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->velY <= 0.0f) {
        enter(obj, PlayerState::Falling);
    }
}

void LogicContainer::tickFalling(GameObject& obj, float /*dt*/) {
    auto* physics = obj.getComponent<Physics>();
    if (!physics) return;

    if (physics->onGround) {
        if (anyMoveHeld()) enter(obj, PlayerState::Walking);
        else               enter(obj, PlayerState::Idle);
    }
}


void LogicContainer::trySpawnBulletNear(GameObject& obj, GameObjectManager& manager) {
    auto* playerT = obj.getComponent<Transform>();
    auto* fsm = obj.getComponent<StateMachine>();
    if (!playerT || !fsm) return;

    GameObject* bullet = manager.getGameObject("bullet");
    if (!bullet) return;

    Transform* t = bullet->getComponent<Transform>();
    Physics* p = bullet->getComponent<Physics>();
    if (!t || !p) return;

    const bool right = fsm->facingRight;
    t->x = playerT->x + (right ? 0.8f : -0.8f);
    t->y = playerT->y + 0.3f;
    t->z = 0.02f;
    t->scaleX = t->scaleY = t->scaleZ = 0.5f;

    p->velX = right ? 6.0f : -6.0f;
    p->velY = 5.0f;
    p->onGround = false;


    UI::addDebugMessage("[LogicContainer] player fired bullet");
    std::cout << "[LogicContainer] player fired bullet\n";


}


void LogicContainer::update(GameObject& obj, float dt, GameObjectManager& manager) {
    //bstd::cout << "[LogicContainer] update for " << obj.getObjectName() << "\n";

    auto* fsm = obj.getComponent<StateMachine>();
    if (!fsm) return;

    fsm->stateTime += dt;

    // Quick input-driven action that needs the manager (shoot)
    if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE)) {
        trySpawnBulletNear(obj, manager);

    }

    switch (fsm->state) {
    case PlayerState::Idle:     tickIdle(obj, dt);     break;
    case PlayerState::Walking:  tickWalking(obj, dt);  break;
    case PlayerState::Jumping:  tickJumping(obj, dt);  break;
    case PlayerState::Falling:  tickFalling(obj, dt);  break;
    default: break;
    }
}
