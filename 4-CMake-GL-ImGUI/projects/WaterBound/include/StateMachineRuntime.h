/* Start Header ************************************************************************/
/*!
\file		StateMachineRuntime.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      class-based Finite State Machine (FSM), runtime for the engine. This module provides:
            - class StateMachine: owns states and transitions, updates logic per frame
            - class State: abstract base class for per-state behaviour
            - class Transition: condition/action-based links between states

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

class GameObject;

// ============================================================================
// RuntimeTransition - MUST be defined BEFORE RuntimeState
// ============================================================================

/*!
 * \class RuntimeTransition
 * \brief Directed link from one state to another, guarded by a condition.
 */
class RuntimeTransition
{
public:
    using ConditionFn = std::function<bool(GameObject&)>;
    using ActionFn = std::function<void(GameObject&)>;

    /*!
     * \brief Construct a transition.
     * \param toStateName Destination state name.
     * \param condition   Condition predicate.
     * \param action      Optional action executed right before switching state.
     */
    RuntimeTransition(std::string toStateName, ConditionFn condition, ActionFn action = nullptr)
        : to(std::move(toStateName))
        , cond(std::move(condition))
        , act(std::move(action))
    {
    }

    /*! \brief Returns the target state name. */
    const std::string& getTargetState() const { return to; }

    /*! \brief Evaluates the transition condition. */
    bool checkCondition(GameObject& obj) const { return cond && cond(obj); }

    std::string to;     //!< Destination state name.
    ConditionFn cond;   //!< Condition to trigger transition.
    ActionFn act;       //!< Optional transition action.
};

// ============================================================================
// RuntimeState - Now RuntimeTransition is fully defined
// ============================================================================

/*!
 * \class RuntimeState
 * \brief Abstract base class for a single FSM state.
 *
 * Derive from this class to implement per-state behaviour in a proper
 * class-based manner (rubric compliant).
 */
class RuntimeState
{
public:
    virtual ~RuntimeState() = default;

    /*!
     * \brief Returns a stable, human-readable state name.
     * Used as the key inside RuntimeStateMachine. Must be unique per machine.
     */
    virtual const char* name() const = 0;

    /*! \brief Called once when this state becomes active. (EntryAction) */
    virtual void onEnter(GameObject& obj) { (void)obj; }

    /*! \brief Called every frame while this state is active. (UpdateAction) */
    virtual void onUpdate(GameObject& obj, float dt) = 0;

    /*! \brief Called once when this state is exited. (ExitAction) */
    virtual void onExit(GameObject& obj) { (void)obj; }

    /*! \brief Adds a transition from this state. */
    void addTransition(RuntimeTransition t) {
        m_transitions.push_back(std::move(t));
    }

    /*! \brief Returns all transitions from this state. */
    const std::vector<RuntimeTransition>& getTransitions() const {
        return m_transitions;
    }

protected:
    std::vector<RuntimeTransition> m_transitions; //!< Transitions owned by this state
};

// ============================================================================
// RuntimeStateMachine - FSM container
// ============================================================================

/*!
 * \class RuntimeStateMachine
 * \brief Owns states and updates them per frame. (FSM)
 */
class RuntimeStateMachine
{
public:
    /*! \brief Adds (or replaces) a state in this machine. */
    void addState(std::unique_ptr<RuntimeState> s);

    /*! \brief Sets the initial current state and calls onEnter. */
    void setInitial(const std::string& stateName, GameObject& obj);

    /*! \brief Adds a transition to a specific state. */
    void addTransition(const std::string& fromState, RuntimeTransition t);

    /*! \brief Updates the current state and evaluates transitions. */
    void update(GameObject& obj, float dt);

    /*! \brief Returns the current state's name. */
    const std::string& currentStateName() const { return m_currentName; }

    /*! \brief Returns how long we have been in the current state (seconds). */
    float stateTime() const { return m_stateTime; }

    /*! \brief Returns the initial state name. */
    const std::string& initialStateName() const { return m_initialName; }

private:
    /*! \brief Internal state change helper. */
    void changeState(const std::string& toState, GameObject& obj);

    std::unordered_map<std::string, std::unique_ptr<RuntimeState>> m_states;  //!< All states

    std::string m_initialName;   //!< Initial state name
    std::string m_currentName;   //!< Current state name
    float m_stateTime = 0.0f;    //!< Time in current state
};