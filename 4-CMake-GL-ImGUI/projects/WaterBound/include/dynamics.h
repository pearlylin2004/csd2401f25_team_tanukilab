/* Start Header ************************************************************************/
/*!
\file       dynamics.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      Defines the Force and Dynamics structures along with the
            DynamicsSystem class responsible for handling physics-based
            motion and force integration in the game.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "mathlib.h"
#include <vector>

struct Force
{
    Vector3D direction; // normalized
    float magnitude;
    float lifetime;     // in seconds (-1 for infinite)
    bool active = true;
};

struct Dynamics
{
    Vector3D position;
    Vector3D velocity;
    Vector3D acceleration;

    Vector3D angularVelocity;
    Vector3D angularAcceleration;

    float mass = 1.0f;
    float inverseMass = 1.0f;

    std::vector<Force> forces;
};

class DynamicsSystem
{
public:
    static void AddForce(Dynamics& dyn, const Vector3D& direction, float magnitude, float lifetime = -1.0f);
    static void UpdateForces(Dynamics& dyn, float deltaTime);
    static void Integrate(Dynamics& dyn, float deltaTime, float gravityValue, bool onGround);
};
