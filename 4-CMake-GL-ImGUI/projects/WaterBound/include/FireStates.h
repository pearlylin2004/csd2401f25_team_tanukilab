#pragma once
#include "StateMachineRuntime.h"  // Use the correct header name

// ============================================================================
// FIRE STATES
// ============================================================================

class FireActiveState : public RuntimeState {
public:
    const char* name() const override { return "Active"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};

class FireExtinguishedState : public RuntimeState {
public:
    const char* name() const override { return "Extinguished"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
private:
    float m_fadeTimer = 0.0f;
};