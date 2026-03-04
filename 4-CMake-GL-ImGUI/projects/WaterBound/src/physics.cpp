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
void PhysicsForces::applyDamping(Physics* physics, float friction, float targetVelX, float deltaTime) {
    // Simple velocity damping 
    //physics->damping; // instead of 0.99f

    //physics->dynamics.velocity.x *= physics->damping;
	physics->dynamics.angularVelocity *= physics->dynamics.angularDrag;
   // physics->dynamics.velocity.y *= physics->damping;

    // accel when moving, decel when stopping
    if (targetVelX != 0.f) {
        float accel = physics->onGround ? (physics->groundAccelForce / physics->dynamics.mass) : (physics->airAccelForce / physics->dynamics.mass);

        physics->dynamics.velocity.x += (targetVelX - physics->dynamics.velocity.x) * accel * deltaTime;
    }
    else {
        float decel = (physics->velY <= 0) ? (1.f - friction) : physics->damping;

        physics->dynamics.velocity.x *= pow(decel, deltaTime * 60.0f);

        if (fabsf(physics->dynamics.velocity.x) < 0.5f) {
            physics->dynamics.velocity.x = 0.f;
        }
    }
}
/**
 * @brief Updates object position based on physics
 * Integrates physics forces and updates transform position. 
 */
void PhysicsForces::updatePosition(Transform* transform, Physics* physics, float deltaTime) {
    DynamicsSystem::UpdateForces(physics->dynamics, deltaTime);
    if (!physics->onGround) {
        //DynamicsSystem::Integrate(physics->dynamics, deltaTime, physics->gravity, false);
        float grav = physics->gravity * physics->dynamics.mass; // gravity force F=mg
        if (physics->dynamics.velocity.y < 0.0f)
            grav *= 1.5f;  // faster fall
        else if (std::abs(physics->dynamics.velocity.y) < 1.0f)
            grav *= physics->apexHangMul; // apex hang when near the peak

        // integrate gravity into velocity
        physics->dynamics.velocity.y += grav * deltaTime;

        // terminal velocity
        physics->dynamics.velocity.y = std::max(physics->dynamics.velocity.y, physics->maxFallSpeed);

        // integrate velocity into position
        physics->dynamics.position.x += physics->dynamics.velocity.x * deltaTime;
        physics->dynamics.position.y += physics->dynamics.velocity.y * deltaTime;

        transform->x = physics->dynamics.position.x;
        transform->y = physics->dynamics.position.y;
    }
    else {
        // sync dynamics from transform
        physics->dynamics.position.x = transform->x;
        physics->dynamics.position.y = transform->y;
        physics->dynamics.velocity.y = 0.0f;
        //physics->velY = 0.0f;
    }

    // Integrate angular velocity into rotation (degrees/sec)
    if (!physics->freezeRotation) {
        DynamicsSystem::IntegrateAngular(physics->dynamics, deltaTime);
        transform->rotation += physics->dynamics.angularVelocity * deltaTime;
    }

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
    Transform* transform = object->getComponent<Transform>();

    physics->dynamics.position.x = transform->x;
    physics->dynamics.position.y = transform->y;

    // direct velocity impulse
    float jumpVel = physics->jumpForce / physics->dynamics.mass;
    physics->dynamics.velocity.y = jumpVel;
    physics->velY = jumpVel;
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
void PhysicsForces::shoot(GameObject* bullet, const Transform* origin, const Vector2D& targetPos) {
    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Transform* t = bullet->getComponent<Transform>();
    Physics* p = bullet->getComponent<Physics>();

    //adjust spawn X based on flip
    float barrelX = origin->x;
    float barrelY = origin->y;

    // Spawn at bullet position
    float dx = targetPos.x - barrelX;
    float dy = targetPos.y - barrelY;

    //normalize direction
    float magnitude = sqrtf(dx * dx + dy * dy);
    float dirX = dx / magnitude;
    float dirY = dy / magnitude;

    // Spawn at barrel tip
    t->x = barrelX;
    t->y = barrelY;
    t->z = 0.0f;
    p->dynamics.position.x = barrelX;
    p->dynamics.position.y = barrelY;

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

    p->dynamics.forces.clear();

    p->lifeTimer = 0.0f;
    p->alive = false;

    DebugLog::addMessage("Bullet deactivated and returned to pool", DebugMode::PlaySimul);
}

void Physics::recalculateInertia(const Vector2D& colliderSize, shape colliderType) {
    if (inertiaOverride > 0.0f) {
        dynamics.inertia = inertiaOverride;
        return;
    }
    if (colliderType == shape::circle) {
        float radius = colliderSize.x / 2.0f;
        dynamics.inertia = 0.5f * dynamics.mass * radius * radius;
    }
    else {
        float w = colliderSize.x;
        float h = colliderSize.y;
        dynamics.inertia = (1.0f / 12.0f) * dynamics.mass * (w * w + h * h);
    }
}