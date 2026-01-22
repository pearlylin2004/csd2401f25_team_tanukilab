/* Start Header ************************************************************************/
/*!
\file		TriggerActionRuntime.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief		Implements the Trigger-Action system for environmental gameplay logic.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "TriggerActionRuntime.h"
#include "GameObject.h"
#include "Component.h"
#include "Systems.h"
#include <iostream>
#include <cmath>

// ============================================================================
// RULE
// ============================================================================

Rule::Rule(std::unique_ptr<Trigger> t, std::unique_ptr<Action> a, bool once)
    : m_trigger(std::move(t))
    , m_action(std::move(a))
    , m_fireOnce(once)
    , m_fired(false)
{
}

void Rule::update(GameObject& obj) {
    if (m_fireOnce && m_fired) return;

    if (m_trigger && m_trigger->check(obj)) {
        if (m_action) {
            m_action->execute(obj);
        }
        if (m_fireOnce) {
            m_fired = true;
        }
    }
}

void Rule::reset() {
    m_fired = false;
}

// ============================================================================
// TRIGGER ACTION SYSTEM
// ============================================================================

void TriggerActionSystem::addRule(GameObject* obj, std::unique_ptr<Rule> rule) {
    if (!obj || !rule) return;
    m_rules[obj].push_back(std::move(rule));
}

void TriggerActionSystem::update(GameObject& obj) {
    auto it = m_rules.find(&obj);
    if (it == m_rules.end()) return;

    for (auto& rule : it->second) {
        if (rule) {
            rule->update(obj);
        }
    }
}

void TriggerActionSystem::clearRules(GameObject* obj) {
    if (!obj) return;
    m_rules.erase(obj);
}

void TriggerActionSystem::resetRules(GameObject* obj) {
    if (!obj) return;

    auto it = m_rules.find(obj);
    if (it == m_rules.end()) return;

    for (auto& rule : it->second) {
        if (rule) {
            rule->reset();
        }
    }
}

// ============================================================================
// COMMON TRIGGERS
// ============================================================================

CollisionTrigger::CollisionTrigger(std::string tag)
    : m_targetTag(std::move(tag))
{
}

bool CollisionTrigger::check(GameObject& obj) const {
    // This requires collision system support
    // For now, return false (you'll need to implement collision tag checking)
    (void)obj;

    // PLACEHOLDER: You would check if obj is currently colliding with 
    // any object that has m_targetTag

    // Example pseudo-code:
    // CollisionInfo* collision = obj.getComponent<CollisionInfo>();
    // if (collision && collision->isCollidingWith(m_targetTag)) {
    //     return true;
    // }

    return false;
}

PrefabIDTrigger::PrefabIDTrigger(std::string prefabID)
    : m_prefabID(std::move(prefabID))
{
}

bool PrefabIDTrigger::check(GameObject& obj) const {
    return obj.getObjectPrefabID() == m_prefabID;
}

PositionTrigger::PositionTrigger(float x, float y, float radius)
    : m_x(x), m_y(y), m_radius(radius)
{
}

bool PositionTrigger::check(GameObject& obj) const {
    Transform* t = obj.getComponent<Transform>();
    if (!t) return false;

    float dx = t->x - m_x;
    float dy = t->y - m_y;
    float distSq = dx * dx + dy * dy;

    return distSq <= (m_radius * m_radius);
}

// ============================================================================
// COMMON ACTIONS
// ============================================================================

void EnterDeadAction::execute(GameObject& obj) {
    if (StateMachine* sm = obj.getComponent<StateMachine>()) {
        sm->state = PlayerState::Dead;
        sm->stateTime = 0.0f;

        std::string msg = "[TriggerAction] " + obj.getObjectName() + " killed by hazard";
        DebugLog::addMessage(msg, DebugMode::PlaySimul);
        std::cout << msg << std::endl;
    }
}

PlaySoundOnceAction::PlaySoundOnceAction(std::string soundFile)
    : m_soundFile(std::move(soundFile))
{
}

void PlaySoundOnceAction::execute(GameObject& obj) {
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* ch = audio->getDefaultChannel();
        if (ch) {
            ch->audioFile = m_soundFile;
            ch->loop = false;
            ch->volume = 1.0f;
            ch->isPendingPlay = true;
        }
    }
}

DebugLogAction::DebugLogAction(std::string message)
    : m_message(std::move(message))
{
}

void DebugLogAction::execute(GameObject& obj) {
    std::string msg = "[TriggerAction] " + obj.getObjectName() + ": " + m_message;
    DebugLog::addMessage(msg, DebugMode::PlaySimul);
    std::cout << msg << std::endl;
}

LambdaAction::LambdaAction(ActionFn fn)
    : m_fn(std::move(fn))
{
}

void LambdaAction::execute(GameObject& obj) {
    if (m_fn) {
        m_fn(obj);
    }
}