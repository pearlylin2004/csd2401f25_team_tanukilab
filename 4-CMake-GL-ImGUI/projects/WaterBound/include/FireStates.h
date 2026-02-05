/* Start Header ************************************************************************/
/*!
\file       FireStates.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       February, 5th, 2026
\brief      Declares the Fire FSM states (Active and Extinguished) as part of the
            class-based finite state machine implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "StateMachineRuntime.h"  

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