#pragma once
#include "StateMachineRuntime.h"

// ============================================================================
// WATER REFILL STATES  
// ============================================================================

class RefillFullState : public RuntimeState {
public:
    const char* name() const override { return "Full"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};

class RefillEmptyState : public RuntimeState {
public:
    const char* name() const override { return "Empty"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};
