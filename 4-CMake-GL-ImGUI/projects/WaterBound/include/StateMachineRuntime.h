/* Start Header ************************************************************************/
/*!
\file		StateMachineRuntime.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declares a rubric-compliant, class-based Finite State Machine (FSM)
            runtime for the engine. This module provides:
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

class GameObject;

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

    /*! \brief Called once when this state becomes active. */
    virtual void onEnter(GameObject& obj) { (void)obj; }

    /*! \brief Called every frame while this state is active. */
    virtual void onUpdate(GameObject& obj, float dt) = 0;

    /*! \brief Called once when this state is exited. */
    virtual void onExit(GameObject& obj) { (void)obj; }
};

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

    std::string to;     //!< Destination state name.
    ConditionFn cond;   //!< Condition to trigger transition.
    ActionFn act;       //!< Optional transition action.
};

/*!
 * \class RuntimeStateMachine
 * \brief Owns states + transitions and updates them per frame.
 */
class RuntimeStateMachine
{
public:
    /*! \brief Adds (or replaces) a state in this machine. */
    void addState(std::unique_ptr<RuntimeState> s);

    /*! \brief Sets the initial current state and calls onEnter. */
    void setInitial(const std::string& stateName, GameObject& obj);

    /*! \brief Adds a transition OUT of a given state. */
    void addTransition(const std::string& fromState, RuntimeTransition t);

    /*! \brief Updates the current state and evaluates transitions. */
    void update(GameObject& obj, float dt);

    /*! \brief Returns the current state's name. */
    const std::string& currentStateName() const { return m_currentName; }

    /*! \brief Returns how long we have been in the current state (seconds). */
    float stateTime() const { return m_stateTime; }

private:
    /*! \brief Internal state change helper. */
    void changeState(const std::string& toState, GameObject& obj);

    std::unordered_map<std::string, std::unique_ptr<RuntimeState>> m_states;
    std::unordered_map<std::string, std::vector<RuntimeTransition>> m_transitions;

    std::string m_currentName;
    float m_stateTime = 0.0f;
};