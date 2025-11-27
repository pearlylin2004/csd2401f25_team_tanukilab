/* Start Header ************************************************************************/
/*!
\file       physics.cpp
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
#include "physics.h"
#include "input.h"
#include "dynamics.h"

void PhysicsForces::applyGravity(Physics* physics, float deltaTime) {
    // Gravity as a constant downward force: F = m * g
    Vector3D gravityForce = { 0.0f, physics->gravity * physics->dynamics.mass, 0.0f };
    DynamicsSystem::AddForce(physics->dynamics, gravityForce, 1.0f);
}

void PhysicsForces::applyDamping(Physics* physics) {
    // Simple velocity damping 
    physics->damping; // instead of 0.99f

    physics->dynamics.velocity.x *= physics->damping;
   // physics->dynamics.velocity.y *= physics->damping;
}
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

void PhysicsForces::shoot(GameObject* bullet, const Transform* origin) {
    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Transform* t = bullet->getComponent<Transform>();
    Physics* p = bullet->getComponent<Physics>();

    // Spawn at player position
    t->x = origin->x;
    t->y = origin->y;
    t->z = 0.0f;
    //t->scaleX = 0.5f;
    //t->scaleY = 0.5f;
    //t->scaleZ = 0.5f;
    //t->rotation = 0.0f;

    p->dynamics.position.x = origin->x;
    p->dynamics.position.y = origin->y;

    if (origin->flipX) {
        p->dynamics.velocity.x = -p->moveSpeed;
    }
    else {
        p->dynamics.velocity.x = p->moveSpeed;
    }

    // Set bullet velocity to move horizontally along x-axis
    //p->dynamics.velocity.x = 10.0f; // speed along x
    p->dynamics.velocity.y = 0.0f;  // no vertical motion

    p->lifeTimer = 0.0f;
    p->alive = true;

    // Optional: clear forces so bullet is not affected by gravity
    p->dynamics.forces.clear();

    DebugLog::addMessage("Bullet fired horizontally!", DebugMode::PlaySimul);
}


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