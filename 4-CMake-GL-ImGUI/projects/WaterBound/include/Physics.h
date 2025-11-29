/* Start Header ************************************************************************/
/*!
\file       Physics.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      Implements the PhysicsForces class responsible for applying
            physics-based behaviors such as gravity, damping, jumping,

            The system integrates with the DynamicsSystem to handle forces
            and updates object positions and velocities over time.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "gameObject.h"
#include "dynamics.h"
#include "GameObjectManager.h"

class GameObjectManager;

// Struct to handle jump and bullet physics
struct PhysicsForces {
    /*static void applyGravity(Physics* physics, float deltaTime);*/
    static void applyGravity(Physics* physics);
    static void applyDamping(Physics* physics);

    static void updatePosition(Transform* transform, Physics* physics, float deltaTime);

    static void jump(GameObject* object);           // Apply jump force
    static void shoot(GameObject* bullet, const Transform* origin);  // Shoot bullet from origin

    /*static void applyDynamics(Physics* physics, float deltaTime);*/

    static GameObject* findAvailableBullet(GameObjectManager& manager);
    static void deactivateBullet(GameObject* bullet);
};