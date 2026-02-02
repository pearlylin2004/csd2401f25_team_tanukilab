/* Start Header ************************************************************************/
/*!
\file		LogicContainer.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the LogicContainer using class-based FSM and trigger-action system.
            

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LogicContainer.h"
#include "PlayerStates.h"
#include "GameObject.h"
#include "input.h"
#include "Systems.h"
#include "Component.h"
#include "GameObjectManager.h"
#include <iostream>

// ============================================================================
// Helper Functions
// ============================================================================

/*!
 * \brief Checks if any movement key (A/D) is currently held.
 * \return True if movement input detected.
 */
static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

/*!
 * \brief Checks if this GameObject is a special prefab that shouldn't walk.
 * \param obj The GameObject to check.
 * \return True if it's a blocked prefab.
 */
static inline bool isBlockedPrefab(GameObject& obj) {
    std::string prefabID = obj.getObjectPrefabID();
    return prefabID == "eaa2ba42-971a-413b-b8c4-99b2f5ab674d" ||
        prefabID == "f04819ff-270c-41b4-8387-73382eb85103";
}

// ============================================================================
// GET OR CREATE RUNTIME FSM
// ============================================================================

RuntimeStateMachine* LogicContainer::getOrCreateMachine(GameObject& obj) {
    // Check if machine already exists
    auto it = m_machines.find(&obj);
    if (it != m_machines.end()) {
        return it->second.get();
    }

    // Create new machine
    auto machine = std::make_unique<RuntimeStateMachine>();

    // ===== CREATE STATES =====
    auto idleState = std::make_unique<IdleState>();
    auto walkingState = std::make_unique<WalkingState>();
    auto jumpingState = std::make_unique<JumpingState>();
    auto fallingState = std::make_unique<FallingState>();
    auto deadState = std::make_unique<DeadState>();

    // ===== TRANSITIONS FROM IDLE (added to IdleState) =====
    idleState->addTransition(RuntimeTransition(
        "Walking",
        [](GameObject& o) {
            return anyMoveHeld() && !isBlockedPrefab(o);
        }
    ));

    idleState->addTransition(RuntimeTransition(
        "Jumping",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && InputHandler::isKeyTriggered(GLFW_KEY_SPACE) && p->onGround;
        },
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            if (p) {
                p->velY = p->jumpForce;
                p->onGround = false;
            }
        }
    ));

    // ===== TRANSITIONS FROM WALKING (added to WalkingState) =====
    walkingState->addTransition(RuntimeTransition(
        "Idle",
        [](GameObject& o) {
            StateMachine* fsm = o.getComponent<StateMachine>();
            return !anyMoveHeld() && (!fsm || !fsm->fixState);
        }
    ));

    walkingState->addTransition(RuntimeTransition(
        "Falling",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && !p->onGround && p->velY < 0.0f;
        }
    ));

    walkingState->addTransition(RuntimeTransition(
        "Jumping",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && InputHandler::isKeyTriggered(GLFW_KEY_SPACE) && p->onGround;
        },
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            if (p) {
                p->velY = p->jumpForce;
                p->onGround = false;
            }
        }
    ));

    // ===== TRANSITIONS FROM JUMPING (added to JumpingState) =====
    jumpingState->addTransition(RuntimeTransition(
        "Falling",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->velY <= 0.0f;
        }
    ));

    // ===== TRANSITIONS FROM FALLING (added to FallingState) =====
    fallingState->addTransition(RuntimeTransition(
        "Walking",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->onGround && anyMoveHeld();
        },
        [](GameObject& o) {
            if (AudioComponent* audio = o.getComponent<AudioComponent>()) {
                AudioChannel* landing = audio->getOrCreateChannel("landing");
                if (landing) {
                    landing->audioFile = "assets/audio/landing.wav";
                    landing->loop = false;
                    landing->volume = 1.0f;
                    landing->isPendingPlay = true;
                }
            }
        }
    ));

    fallingState->addTransition(RuntimeTransition(
        "Idle",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->onGround && !anyMoveHeld();
        },
        [](GameObject& o) {
            if (AudioComponent* audio = o.getComponent<AudioComponent>()) {
                AudioChannel* landing = audio->getOrCreateChannel("landing");
                if (landing) {
                    landing->audioFile = "assets/audio/landing.wav";
                    landing->loop = false;
                    landing->volume = 1.0f;
                    landing->isPendingPlay = true;
                }
            }
        }
    ));

    // ===== ADD STATES TO MACHINE =====
    machine->addState(std::move(idleState));
    machine->addState(std::move(walkingState));
    machine->addState(std::move(jumpingState));
    machine->addState(std::move(fallingState));
    machine->addState(std::move(deadState));

    // ===== SET INITIAL STATE =====
    machine->setInitial("Idle", obj);

    // Store and return
    RuntimeStateMachine* ptr = machine.get();
    m_machines[&obj] = std::move(machine);
    return ptr;
}

// ============================================================================
// SYNC STATE TO COMPONENT
// ============================================================================

void LogicContainer::syncStateToComponent(GameObject& obj) {
    RuntimeStateMachine* machine = getOrCreateMachine(obj);
    StateMachine* component = obj.getComponent<StateMachine>();

    if (!machine || !component) return;

    // Sync runtime state → component for editor/debug/animation
    std::string current = machine->currentStateName();

    if (current == "Idle") {
        component->state = PlayerState::Idle;
    }
    else if (current == "Walking") {
        component->state = PlayerState::Walking;
    }
    else if (current == "Jumping") {
        component->state = PlayerState::Jumping;
    }
    else if (current == "Falling") {
        component->state = PlayerState::Falling;
    }
    else if (current == "Dead") {
        component->state = PlayerState::Dead;
    }

    component->stateTime = machine->stateTime();
}

// ============================================================================
// MAIN UPDATE
// ============================================================================

void LogicContainer::update(GameObject& obj, float dt) {
    // Update the runtime FSM
    RuntimeStateMachine* machine = getOrCreateMachine(obj);
    if (!machine) return;

    machine->update(obj, dt);

    // Sync to component for editor/animation
    syncStateToComponent(obj);

    // Update trigger-action rules
    m_triggerActionSystem.update(obj);
}