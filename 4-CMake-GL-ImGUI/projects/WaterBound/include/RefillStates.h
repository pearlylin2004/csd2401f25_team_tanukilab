/* Start Header ************************************************************************/
/*!
\file       RefillStates.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       February, 5th, 2026
\brief      Declares the Water Refill FSM states (Full and Empty) as part of the
            class-based finite state machine implementation for water refill stations.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

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
