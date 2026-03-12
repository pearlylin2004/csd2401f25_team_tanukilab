/* Start Header ************************************************************************/
/*!
\file		LogicContainer.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declares the LogicContainer system responsible for managing class-based
            finite state machines and trigger-action rules.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include "Component.h"
#include "input.h"
#include "StateMachineRuntime.h"
#include "TriggerActionRuntime.h"
#include <unordered_map>
#include <memory>

class GameObject;
class GameObjectManager;

/*!
 * \class LogicContainer
 * \brief Manages class-based FSMs and trigger-action rules for gameplay logic.
 *
 * This system:
 * - Creates and updates RuntimeStateMachine instances per GameObject
 * - Syncs runtime FSM state back to StateMachine component for editor/animation
 * - Manages trigger-action rules for environmental interactions
 */
class LogicContainer {
public:

    // Set to true to clear all FSM machines on the next update (used by replay rewind)
    static bool s_replayResetNeeded;

    /*!
     * \brief Destroys all cached FSM machines so they are recreated in
     *        their initial state on the next update tick.
     */
    void clearAllMachines() { m_machines.clear(); }

    /*!
     * \brief Main per-frame update function for a GameObject.
     *
     * Updates the runtime FSM, syncs state to component, and processes
     * trigger-action rules.
     *
     * \param obj      Reference to the GameObject whose logic is being updated.
     * \param dt       Delta time for the current frame.
     */
    void update(GameObject& obj, float dt);

    /*!
     * \brief Get or create the RuntimeStateMachine for this GameObject.
     *
     * If the machine doesn't exist, builds a new one with all states and transitions.
     *
     * \param obj The GameObject to get/create machine for.
     * \return Pointer to the RuntimeStateMachine (never null after call).
     */
    RuntimeStateMachine* getOrCreateMachine(GameObject& obj);

    /*!
     * \brief Get the TriggerActionSystem for managing rules.
     * \return Reference to the trigger-action system.
     */
    TriggerActionSystem& getTriggerActionSystem() { return m_triggerActionSystem; }

    void updateFire(GameObject& obj, float dt);
    void updateRefill(GameObject& obj, float dt);
    RuntimeStateMachine* getOrCreateFireMachine(GameObject& obj);
    RuntimeStateMachine* getOrCreateRefillMachine(GameObject& obj);

private:

    /*!
     * \brief Syncs the runtime FSM state back to the StateMachine component.
     *
     * This allows the editor and animation system to see the current state.
     *
     * \param obj The GameObject to sync.
     */
    void syncStateToComponent(GameObject& obj);

    // Storage for runtime FSMs (one per GameObject)
    std::unordered_map<GameObject*, std::unique_ptr<RuntimeStateMachine>> m_machines;

    // Trigger-action system for environmental logic
    TriggerActionSystem m_triggerActionSystem;
};