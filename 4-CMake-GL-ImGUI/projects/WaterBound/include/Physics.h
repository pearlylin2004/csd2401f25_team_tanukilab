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