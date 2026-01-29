/* Start Header ************************************************************************/
/*!
\file		StateMachineRuntime.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the rubric-compliant, class-based FSM runtime declared in
            StateMachineRuntime.h.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "StateMachineRuntime.h"

#include <iostream>
#include <utility> // for std::move

void RuntimeStateMachine::addState(std::unique_ptr<RuntimeState> s)
{
    if (!s) return;
    m_states[s->name()] = std::move(s);
}

void RuntimeStateMachine::setInitial(const std::string& stateName, GameObject& obj)
{
    m_currentName = stateName;
    m_stateTime = 0.0f;

    auto it = m_states.find(m_currentName);
    if (it != m_states.end())
    {
        it->second->onEnter(obj);
    }
    else
    {
        std::cerr << "[RuntimeStateMachine] Missing initial state: " << m_currentName << "\n";
    }
}

void RuntimeStateMachine::addTransition(const std::string& fromState, RuntimeTransition t)
{
    m_transitions[fromState].push_back(std::move(t));
}

void RuntimeStateMachine::changeState(const std::string& toState, GameObject& obj)
{
    if (toState == m_currentName)
        return;

    // Exit old
    if (!m_currentName.empty())
    {
        auto itOld = m_states.find(m_currentName);
        if (itOld != m_states.end())
            itOld->second->onExit(obj);
    }

    // Switch + reset timer
    m_currentName = toState;
    m_stateTime = 0.0f;

    // Enter new
    auto itNew = m_states.find(m_currentName);
    if (itNew != m_states.end())
    {
        itNew->second->onEnter(obj);
    }
    else
    {
        std::cerr << "[RuntimeStateMachine] Missing destination state: " << m_currentName << "\n";
    }
}

void RuntimeStateMachine::update(GameObject& obj, float dt)
{
    m_stateTime += dt;

    auto it = m_states.find(m_currentName);
    if (it == m_states.end())
        return;

    it->second->onUpdate(obj, dt);

    auto trIt = m_transitions.find(m_currentName);
    if (trIt == m_transitions.end())
        return;

    for (auto& t : trIt->second)
    {
        if (t.cond && t.cond(obj))
        {
            if (t.act) t.act(obj);
            changeState(t.to, obj);
            break; // only one transition per frame
        }
    }
}