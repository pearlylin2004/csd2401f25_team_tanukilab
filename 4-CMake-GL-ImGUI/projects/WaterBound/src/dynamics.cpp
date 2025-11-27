/* Start Header ************************************************************************/
/*!
\file       dynamics.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      Implements the functionality defined in DynamicsSystem.h for
            simulating physics-based motion and force application.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "dynamics.h"

// Add a new force
void DynamicsSystem::AddForce(Dynamics& dyn, const Vector3D& direction, float magnitude, float lifetime) {
    Force f;
    f.direction = Vec3_Normalize(&direction);
    f.magnitude = magnitude;
    f.lifetime = lifetime;
    f.active = true;
    dyn.forces.push_back(f);
}

// Update forces: remove expired forces and reduce lifetime
void DynamicsSystem::UpdateForces(Dynamics& dyn, float deltaTime) {
    for (auto it = dyn.forces.begin(); it != dyn.forces.end(); ) {
        if (!it->active || (it->lifetime <= 0.0f)) {
            it = dyn.forces.erase(it);
        }
        else {
            if (it->lifetime > 0.0f)
                it->lifetime -= deltaTime;
            ++it;
        }
    }
}

// Integrate motion using forces and gravity
void DynamicsSystem::Integrate(Dynamics& dyn, float deltaTime, float gravityValue, bool canMove) {
    Vector3D totalForce = Vec3_Zero();

    for (const auto& f : dyn.forces) {
        if (f.active) {
            Vector3D temp = Vec3_Scale(&f.direction, f.magnitude);
            totalForce = Vec3_Add(&totalForce, &temp);
        }
    }

    if (!canMove) {
        Vector3D gravityForce = { 0.0f, gravityValue * dyn.mass, 0.0f };
        totalForce = Vec3_Add(&totalForce, &gravityForce);
    }

    if (dyn.mass == 0.f) dyn.inverseMass = 0.f;
    else dyn.inverseMass = 1.f / dyn.mass;

    dyn.acceleration = Vec3_Scale(&totalForce, dyn.inverseMass);
    Vector3D deltaVel = Vec3_Scale(&dyn.acceleration, deltaTime);
    dyn.velocity = Vec3_Add(&dyn.velocity, &deltaVel);

    Vector3D deltaPos = Vec3_Scale(&dyn.velocity, deltaTime);
    dyn.position = Vec3_Add(&dyn.position, &deltaPos);
}



