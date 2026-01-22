/* Start Header ************************************************************************/
/*!
\file		PlayerStates.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		November, 5th, 2025
\brief      Declares class-based state implementations for the player FSM.
            Each state (Idle, Walking, Jumping, Falling, Dead) is a proper
            RuntimeState subclass with enter/update/exit behavior.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include "StateMachineRuntime.h"

// Forward declarations
class GameObject;

/*!
 * \class IdleState
 * \brief Player is standing still, waiting for input.
 */
class IdleState : public RuntimeState {
public:
    const char* name() const override { return "Idle"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};

/*!
 * \class WalkingState
 * \brief Player is moving horizontally (A/D keys held).
 */
class WalkingState : public RuntimeState {
public:
    const char* name() const override { return "Walking"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
    void onExit(GameObject& obj) override;
};

/*!
 * \class JumpingState
 * \brief Player is ascending after jump input.
 */
class JumpingState : public RuntimeState {
public:
    const char* name() const override { return "Jumping"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};

/*!
 * \class FallingState
 * \brief Player is descending (no ground contact, negative Y velocity).
 */
class FallingState : public RuntimeState {
public:
    const char* name() const override { return "Falling"; }
    void onUpdate(GameObject& obj, float dt) override;
};

/*!
 * \class DeadState
 * \brief Player has died (collision with hazard, etc.).
 */
class DeadState : public RuntimeState {
public:
    const char* name() const override { return "Dead"; }
    void onEnter(GameObject& obj) override;
    void onUpdate(GameObject& obj, float dt) override;
};