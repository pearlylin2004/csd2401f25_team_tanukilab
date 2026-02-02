/* Start Header ************************************************************************/
/*!
\file       physics.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       November, 7th, 2025
\brief      implements the PhysicsForces struct methods,handles
            force application, object motion updates, and gameplay mechanics
            like jumping and shooting

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "physics.h"
#include "input.h"
#include "dynamics.h"
/**
 * @brief Applies constant gravitational force to a physics object
 *
 * Calculates gravity force using F = m * g and adds force
 * to the dynamics, the force has infinite lifetime (-1.0f).
 */
//void PhysicsForces::applyGravity(Physics* physics, float deltaTime) {
void PhysicsForces::applyGravity(Physics* physics) {
    // Gravity as a constant downward force: F = m * g
    Vector3D gravityForce = { 0.0f, physics->gravity * physics->dynamics.mass, 0.0f };
    DynamicsSystem::AddForce(physics->dynamics, gravityForce, 1.0f);
}
/**
 * @brief Applies velocity damping to simulate friction/air resistance
 * Reduces horizontal velocity over time using damping
 */
void PhysicsForces::applyDamping(Physics* physics) {
    // Simple velocity damping 
    physics->damping; // instead of 0.99f

    physics->dynamics.velocity.x *= physics->damping;
   // physics->dynamics.velocity.y *= physics->damping;
}
/**
 * @brief Updates object position based on physics
 * Integrates physics forces and updates transform position. 
 */
void PhysicsForces::updatePosition(Transform* transform, Physics* physics, float deltaTime) {
    DynamicsSystem::UpdateForces(physics->dynamics, deltaTime);
    //DynamicsSystem::Integrate(physics->dynamics, deltaTime);
    if (!physics->onGround) {
        Vector3D gravity = { 0.0f, physics->gravity * physics->dynamics.mass, 0.0f };
        DynamicsSystem::Integrate(physics->dynamics, deltaTime, physics->gravity, false);
    }
    else {
        physics->dynamics.velocity.y = 0.0f;
        physics->velY = 0.0f;

    }
    transform->x = physics->dynamics.position.x;
    transform->y = physics->dynamics.position.y;

   
    physics->velX = physics->dynamics.velocity.x;
    physics->velY = physics->dynamics.velocity.y;

}
/**
 * @brief Applies jump force to make an object jump
 *
 * Applies an upward force to the object's dynamics system 
 */
void PhysicsForces::jump(GameObject* object) {
    if (!object->hasComponent<Physics>() || !object->hasComponent<Transform>())
        return;

    Physics* physics = object->getComponent<Physics>();


    // Apply upward 
    Vector3D jumpDir = { 0.0f, 1.0f, 0.0f };
    DynamicsSystem::AddForce(physics->dynamics, jumpDir, physics->jumpForce, 0.1f);
    //physics->dynamics.velocity.y = 5.0f; // Instant upward velocity
    physics->onGround = false;
    //physics->canMove = true;
    DebugLog::addMessage("Player jumped!", DebugMode::PlaySimul);
}
/**
 * @brief Initializes and launches a bullet from origin (player)
 *
 * Positions bullet at origin, sets velocity based on origin direction,
 * and configures bullet properties for horizontal movement
 */
//void PhysicsForces::shoot(GameObject* bullet, const Transform* origin, const Vector2D& targetPos) {
//    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
//        return;
//
//    Transform* t = bullet->getComponent<Transform>();
//    Physics* p = bullet->getComponent<Physics>();
//
//    // Spawn at player position
//    t->x = origin->x;
//    t->y = origin->y;
//    t->z = 0.0f;
//
//    p->dynamics.position.x = origin->x;
//    p->dynamics.position.y = origin->y;
//
//    // Calculate displacement to target
//    float dx = targetPos.x - origin->x;
//    float dy = targetPos.y - origin->y;
//    float horizontalDist = fabsf(dx);
//
//    // Prevent shooting at self
//    if (horizontalDist < 0.5f) {
//        p->dynamics.velocity.x = origin->flipX ? -p->moveSpeed : p->moveSpeed;
//        p->dynamics.velocity.y = p->moveSpeed * 0.5f;
//        p->lifeTimer = 0.0f;
//        p->alive = true;
//        return;
//    }
//
//    // Gravity 
//    float g = fabsf(p->gravity);
//
//    float arcHeightFactor = 0.4f; // Adjust between 0.2 - 0.8
//    float desiredPeakHeight = horizontalDist * arcHeightFactor;
//
//    // Ensure minimum arc height
//    desiredPeakHeight = fmaxf(desiredPeakHeight, 2.0f);
//
//    float estimatedTime = sqrtf(2.0f * desiredPeakHeight / g) * 2.0f; // time up + time down for symmetric/ better arc
//
//    if (dy > 0) {
//        estimatedTime *= 1.1f; 
//    }
//    else if (dy < -desiredPeakHeight) {
//        estimatedTime *= 0.9f;
//    }
//
//    // Calculate velocities
//    float initialVelX = dx / estimatedTime;
//
//
//    float initialVelY = (dy + 0.5f * g * estimatedTime * estimatedTime) / estimatedTime;
//
//    // Set velocity components
//    p->dynamics.velocity.x = initialVelX;
//    p->dynamics.velocity.y = initialVelY;
//
//    float maxVel = p->moveSpeed * 3.0f;
//    if (fabsf(p->dynamics.velocity.x) > maxVel) {
//        p->dynamics.velocity.x = (p->dynamics.velocity.x > 0) ? maxVel : -maxVel;
//    }
//    if (fabsf(p->dynamics.velocity.y) > maxVel) {
//        p->dynamics.velocity.y = (p->dynamics.velocity.y > 0) ? maxVel : -maxVel;
//    }
//
//    // Update flipX direction
//    t->flipX = (dx < 0.0f);
//
//    p->lifeTimer = 0.0f;
//    p->alive = true;
//
//    DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
//}

void PhysicsForces::shoot(GameObject* bullet, const Transform* origin, const Vector2D& targetPos) {
    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Transform* t = bullet->getComponent<Transform>();
    Physics* p = bullet->getComponent<Physics>();

    // Spawn at player position
    t->x = origin->x;
    t->y = origin->y;
    t->z = 0.0f;

    p->dynamics.position.x = origin->x;
    p->dynamics.position.y = origin->y;

    // Calculate direction vector from player to mouse
    float dx = targetPos.x - origin->x;
    float dy = targetPos.y - origin->y;
    float magnitude = sqrtf(dx * dx + dy * dy);

    // Prevent shooting at self (fallback to facing direction)
    if (magnitude < 0.5f) {
        dx = origin->flipX ? -1.0f : 1.0f;
        dy = 0.0f;
        magnitude = 1.0f;
    }

    // Normalize direction and scale by bullet speed
    float dirX = dx / magnitude;
    float dirY = dy / magnitude;

    float bulletSpeed = p->moveSpeed * 2.0f;  // Adjust multiplier as needed
    p->dynamics.velocity.x = dirX * bulletSpeed;
    p->dynamics.velocity.y = dirY * bulletSpeed;

    // Update flipX based on direction
    t->flipX = (dx < 0.0f);

    // Reset timers
    p->lifeTimer = 0.0f;
    p->gravityDelayTimer = p->gravityDelay;  // Initialize gravity immunity
    p->alive = true;

    DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
}

/**
 * @brief Finds an available bullet from the object pool
 *
 * Searches through all game objects to find inactive bullets
 * following the naming pattern "bullet" followed by numbers.
 */
GameObject* PhysicsForces::findAvailableBullet(GameObjectManager& manager) { 

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    for (GameObject* obj : gameObjects) {
        std::string name = obj->getObjectName();


        if (name.find("bullet") == 0) {

            bool isNumberedBullet = true;
            if (name.length() > 6) {
                for (size_t i = 6; i < name.length(); i++) {
                    if (!std::isdigit(name[i])) {
                        isNumberedBullet = false;
                        break;
                    }
                }
            }
            else {
                isNumberedBullet = false;
            }

            if (isNumberedBullet) {
                if (!obj->hasComponent<Physics>() || !obj->hasComponent<Transform>())
                    continue;

                Physics* p = obj->getComponent<Physics>();


                if (!p->alive) {
                    return obj;
                }
            }
        }
    }

    return nullptr;
}
/**
 * @brief Deactivates a bullet and resets it for object pooling
 *
 * Resets bullet to its original state (position and velocity) and
 * marks it as available for reuse
 */
void PhysicsForces::deactivateBullet(GameObject* bullet) { 

    if (!bullet || !bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Physics* p = bullet->getComponent<Physics>();
    Transform* t = bullet->getComponent<Transform>();

    // Store original state when first deactivated
    if (!p->isOriginalStateSet) {
        p->originalPos = { t->x, t->y };
        p->originalVel = { p->dynamics.velocity.x, p->dynamics.velocity.y };
        p->isOriginalStateSet = true;
    }

    // Reset to original state instead of hardcoded values
    t->x = p->originalPos.x;
    t->y = p->originalPos.y;

    p->dynamics.position.x = p->originalPos.x;
    p->dynamics.position.y = p->originalPos.y;
    p->dynamics.velocity.x = p->originalVel.x;
    p->dynamics.velocity.y = p->originalVel.y;

    p->lifeTimer = 0.0f;
    p->alive = false;

    DebugLog::addMessage("Bullet deactivated and returned to pool", DebugMode::PlaySimul);
}