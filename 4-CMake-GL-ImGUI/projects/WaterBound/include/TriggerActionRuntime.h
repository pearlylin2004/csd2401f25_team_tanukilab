/* Start Header ************************************************************************/
/*!
\file		TriggerActionRuntime.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declares a Trigger-Action system for environmental interactions.
            Provides class Trigger, class Action, class Rule, and 
            class TriggerActionSystem to handle non-FSM gameplay logic
            (hazards, checkpoints, pickups, etc.).

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

class GameObject;

/*!
 * \class Trigger
 * \brief Abstract base class for a condition/trigger check.
 */
class Trigger {
public:
    virtual ~Trigger() = default;
    
    /*!
     * \brief Check if this trigger condition is met.
     * \param obj The GameObject to check.
     * \return True if triggered, false otherwise.
     */
    virtual bool check(GameObject& obj) const = 0;
};

/*!
 * \class Action
 * \brief Abstract base class for an action to execute.
 */
class Action {
public:
    virtual ~Action() = default;
    
    /*!
     * \brief Execute this action on the given GameObject.
     * \param obj The GameObject to act upon.
     */
    virtual void execute(GameObject& obj) = 0;
};

/*!
 * \class Rule
 * \brief Pairs a Trigger with an Action. Optionally fires only once.
 */
class Rule {
public:
    /*!
     * \brief Construct a Rule.
     * \param t Unique pointer to a Trigger.
     * \param a Unique pointer to an Action.
     * \param once If true, the rule fires only once and then deactivates.
     */
    Rule(std::unique_ptr<Trigger> t, std::unique_ptr<Action> a, bool once = false);
    
    /*!
     * \brief Update the rule: check trigger, execute action if triggered.
     * \param obj The GameObject to check/act on.
     */
    void update(GameObject& obj);
    
    /*!
     * \brief Reset the fired flag (useful for respawn/restart).
     */
    void reset();
    
private:
    std::unique_ptr<Trigger> m_trigger;
    std::unique_ptr<Action> m_action;
    bool m_fireOnce;
    bool m_fired;
};

/*!
 * \class TriggerActionSystem
 * \brief Manages a collection of Rules per GameObject.
 */
class TriggerActionSystem {
public:
    /*!
     * \brief Add a rule to a specific GameObject.
     * \param obj The GameObject to attach the rule to.
     * \param rule Unique pointer to the Rule.
     */
    void addRule(GameObject* obj, std::unique_ptr<Rule> rule);
    
    /*!
     * \brief Update all rules associated with this GameObject.
     * \param obj The GameObject whose rules to update.
     */
    void update(GameObject& obj);
    
    /*!
     * \brief Clear all rules for a specific GameObject.
     * \param obj The GameObject to clear rules from.
     */
    void clearRules(GameObject* obj);
    
    /*!
     * \brief Reset all rules for a GameObject (useful for respawn).
     * \param obj The GameObject whose rules to reset.
     */
    void resetRules(GameObject* obj);
    
private:
    std::unordered_map<GameObject*, std::vector<std::unique_ptr<Rule>>> m_rules;
};

// ============================================================================
// COMMON TRIGGER IMPLEMENTATIONS
// ============================================================================

/*!
 * \class CollisionTrigger
 * \brief Triggers when the object collides with a specific tag.
 */
class CollisionTrigger : public Trigger {
public:
    explicit CollisionTrigger(std::string tag);
    bool check(GameObject& obj) const override;
    
private:
    std::string m_targetTag;
};

/*!
 * \class PrefabIDTrigger
 * \brief Triggers when the object has a specific prefab ID.
 */
class PrefabIDTrigger : public Trigger {
public:
    explicit PrefabIDTrigger(std::string prefabID);
    bool check(GameObject& obj) const override;
    
private:
    std::string m_prefabID;
};

/*!
 * \class PositionTrigger
 * \brief Triggers when object is within a certain distance of a point.
 */
class PositionTrigger : public Trigger {
public:
    PositionTrigger(float x, float y, float radius);
    bool check(GameObject& obj) const override;
    
private:
    float m_x, m_y, m_radius;
};

// ============================================================================
// COMMON ACTION IMPLEMENTATIONS
// ============================================================================

/*!
 * \class EnterDeadAction
 * \brief Transitions the object to the Dead state.
 */
class EnterDeadAction : public Action {
public:
    void execute(GameObject& obj) override;
};

/*!
 * \class PlaySoundOnceAction
 * \brief Plays a sound effect once.
 */
class PlaySoundOnceAction : public Action {
public:
    explicit PlaySoundOnceAction(std::string soundFile);
    void execute(GameObject& obj) override;
    
private:
    std::string m_soundFile;
};

/*!
 * \class DebugLogAction
 * \brief Logs a debug message when triggered.
 */
class DebugLogAction : public Action {
public:
    explicit DebugLogAction(std::string message);
    void execute(GameObject& obj) override;
    
private:
    std::string m_message;
};

/*!
 * \class LambdaAction
 * \brief Wraps a lambda/function as an Action.
 */
class LambdaAction : public Action {
public:
    using ActionFn = std::function<void(GameObject&)>;
    explicit LambdaAction(ActionFn fn);
    void execute(GameObject& obj) override;
    
private:
    ActionFn m_fn;
};