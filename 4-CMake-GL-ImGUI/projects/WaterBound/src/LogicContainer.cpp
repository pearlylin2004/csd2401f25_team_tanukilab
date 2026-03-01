/* Start Header ************************************************************************/
/*!
\file		LogicContainer.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the LogicContainer using class-based FSM and trigger-action system.
            
            FSM Architecture:
            - RuntimeStateMachine: Manages states and transitions
            - RuntimeState: Base class for all states (IdleState, WalkingState, etc.)
            - RuntimeTransition: Defines condition and action for state changes
            
            This is a proper class-based FSM, NOT a switch/enum-based implementation.

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
#include "FireStates.h"
#include "RefillStates.h"
#include <iostream>
#include <random>

// ============================================================================
// Helper Functions
// ============================================================================

static inline bool anyMoveHeld() {
    return InputHandler::isKeyHeld(GLFW_KEY_A) ||
        InputHandler::isKeyHeld(GLFW_KEY_D);
}

static inline bool isBlockedPrefab(GameObject& obj) {
    std::string prefabID = obj.getObjectPrefabID();
    return prefabID == "eaa2ba42-971a-413b-b8c4-99b2f5ab674d" ||
        prefabID == "f04819ff-270c-41b4-8387-73382eb85103";
}

static const std::vector<std::string> s_landingFiles = {
    "assets/audio/landing.wav"
};

static void playRandomLanding(GameObject& o) {
    static std::mt19937 gen{ std::random_device{}() };
    AudioComponent* audio = o.getComponent<AudioComponent>();
    if (!audio || s_landingFiles.empty()) return;

    AudioChannel* landing = audio->getOrCreateChannel("landing");
    if (!landing) return;

    std::uniform_int_distribution<int> dist(0, static_cast<int>(s_landingFiles.size()) - 1);
    std::uniform_real_distribution<float> pitchDist(0.90f, 1.10f);
    std::uniform_real_distribution<float> volDist(0.7f, 1.0f);

    landing->audioFile = s_landingFiles[dist(gen)];
    landing->loop = false;
    landing->pitch = pitchDist(gen);
    landing->volume = volDist(gen);
    landing->isPendingPlay = true;
}

// ============================================================================
// PLAYER FSM FACTORY
// ============================================================================

RuntimeStateMachine* LogicContainer::getOrCreateMachine(GameObject& obj) {
    auto it = m_machines.find(&obj);
    if (it != m_machines.end()) {
        return it->second.get();
    }

    // Create class-based state machine (not enum/switch based)
    auto machine = std::make_unique<RuntimeStateMachine>();

    // Create state objects - each is a class inheriting from RuntimeState
    auto idleState = std::make_unique<IdleState>();
    auto walkingState = std::make_unique<WalkingState>();
    auto jumpingState = std::make_unique<JumpingState>();
    auto fallingState = std::make_unique<FallingState>();
    auto deadState = std::make_unique<DeadState>();

    // Idle -> Walking transition
    idleState->addTransition(RuntimeTransition(
        "Walking",
        [](GameObject& o) { return anyMoveHeld() && !isBlockedPrefab(o); }
    ));

    // Idle -> Jumping transition
    idleState->addTransition(RuntimeTransition(
        "Jumping",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->velY > 0.f;
        }/*,
        [](GameObject& o) {
            if (Physics* p = o.getComponent<Physics>()) {
                p->velY = p->jumpForce;
                p->onGround = false;
            }
        }*/
    ));

    // Walking -> Idle transition
    walkingState->addTransition(RuntimeTransition(
        "Idle",
        [](GameObject& o) {
            StateMachine* fsm = o.getComponent<StateMachine>();
            return !anyMoveHeld() && (!fsm || !fsm->fixState);
        }
    ));

    // Walking -> Falling transition
    walkingState->addTransition(RuntimeTransition(
        "Falling",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && !p->onGround && p->velY < 0.0f;
        }
    ));

    // Walking -> Jumping transition
    walkingState->addTransition(RuntimeTransition(
        "Jumping",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->velY > 0.f;
        }/*,
        [](GameObject& o) {
            if (Physics* p = o.getComponent<Physics>()) {
                p->velY = p->jumpForce;
                p->onGround = false;
            }
        }*/
    ));

    // Jumping -> Falling transition
    jumpingState->addTransition(RuntimeTransition(
        "Falling",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->velY <= 0.0f;
        }
    ));

    // Falling -> Walking transition (with randomized landing sound)
    fallingState->addTransition(RuntimeTransition(
        "Walking",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->onGround && anyMoveHeld();
        },
        [](GameObject& o) {
            playRandomLanding(o);
        }
    ));

    // Falling -> Idle transition (with randomized landing sound)
    fallingState->addTransition(RuntimeTransition(
        "Idle",
        [](GameObject& o) {
            Physics* p = o.getComponent<Physics>();
            return p && p->onGround && !anyMoveHeld();
        },
        [](GameObject& o) {
            playRandomLanding(o);
        }
    ));

    // Add all states to the machine
    machine->addState(std::move(idleState));
    machine->addState(std::move(walkingState));
    machine->addState(std::move(jumpingState));
    machine->addState(std::move(fallingState));
    machine->addState(std::move(deadState));
    machine->setInitial("Idle", obj);

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

    std::string current = machine->currentStateName();

    if (current == "Idle")          component->state = PlayerState::Idle;
    else if (current == "Walking")  component->state = PlayerState::Walking;
    else if (current == "Jumping")  component->state = PlayerState::Jumping;
    else if (current == "Falling")  component->state = PlayerState::Falling;
    else if (current == "Dead")     component->state = PlayerState::Dead;

    component->stateTime = machine->stateTime();
}

// ============================================================================
// MAIN UPDATE
// ============================================================================

void LogicContainer::update(GameObject& obj, float dt) {
    RuntimeStateMachine* machine = getOrCreateMachine(obj);
    if (!machine) return;

    machine->update(obj, dt);
    syncStateToComponent(obj);
    m_triggerActionSystem.update(obj);
}

// ============================================================================
// FIRE FSM FACTORY
// ============================================================================

RuntimeStateMachine* LogicContainer::getOrCreateFireMachine(GameObject& obj) {
    auto it = m_machines.find(&obj);
    if (it != m_machines.end()) {
        return it->second.get();
    }

    auto machine = std::make_unique<RuntimeStateMachine>();

    // Create class-based states for Fire FSM
    auto activeState = std::make_unique<FireActiveState>();
    auto extinguishedState = std::make_unique<FireExtinguishedState>();

    // Active -> Extinguished transition (triggered by bullet collision)
    activeState->addTransition(RuntimeTransition(
        "Extinguished",
        [](GameObject& o) {
            if (VisualLC* vl = o.getComponent<VisualLC>()) {
                for (GameObject* collidedObj : vl->curCollision) {
                    if (!collidedObj) continue;
                    CollisionInfo* c = collidedObj->getComponent<CollisionInfo>();
                    if (c && c->collisionTag == "bullet") {
                        // Debug output - shows FSM transition
                        std::cout << "[FSM] " << o.getObjectName() << ": Active -> Extinguished (bullet hit)" << std::endl;
                        return true;
                    }
                }
            }
            return false;
        }
    ));

    machine->addState(std::move(activeState));
    machine->addState(std::move(extinguishedState));
    machine->setInitial("Active", obj);

    RuntimeStateMachine* ptr = machine.get();
    m_machines[&obj] = std::move(machine);
    return ptr;
}

// ============================================================================
// REFILL FSM FACTORY
// ============================================================================

RuntimeStateMachine* LogicContainer::getOrCreateRefillMachine(GameObject& obj) {
    auto it = m_machines.find(&obj);
    if (it != m_machines.end()) {
        return it->second.get();
    }

    auto machine = std::make_unique<RuntimeStateMachine>();

    // Create class-based states for Refill FSM
    auto fullState = std::make_unique<RefillFullState>();
    auto emptyState = std::make_unique<RefillEmptyState>();

    // Full -> Empty transition (triggered by player collision)
    fullState->addTransition(RuntimeTransition(
        "Empty",
        [](GameObject& o) {
            if (VisualLC* vl = o.getComponent<VisualLC>()) {
                for (GameObject* collidedObj : vl->curCollision) {
                    if (!collidedObj) continue;
                    CollisionInfo* c = collidedObj->getComponent<CollisionInfo>();
                    if (c && c->collisionTag == "player") {
                        // Debug output - shows FSM transition
                        std::cout << "[FSM] " << o.getObjectName() << ": Full -> Empty (player touched)" << std::endl;
                        return true;
                    }
                }
            }
            return false;
        },
        [](GameObject& o) {
            (void)o;
            VisualLC::globalFloatVar["bullets"] = 10.0f;
        }
    ));

    machine->addState(std::move(fullState));
    machine->addState(std::move(emptyState));
    machine->setInitial("Full", obj);

    RuntimeStateMachine* ptr = machine.get();
    m_machines[&obj] = std::move(machine);
    return ptr;
}

// ============================================================================
// FIRE & REFILL UPDATE
// ============================================================================

void LogicContainer::updateFire(GameObject& obj, float dt) {
    RuntimeStateMachine* machine = getOrCreateFireMachine(obj);
    if (machine) machine->update(obj, dt);
}

void LogicContainer::updateRefill(GameObject& obj, float dt) {
    RuntimeStateMachine* machine = getOrCreateRefillMachine(obj);
    if (machine) machine->update(obj, dt);
}