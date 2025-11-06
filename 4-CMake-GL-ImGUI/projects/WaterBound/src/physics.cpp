#include "physics.h"
#include "input.h"
#include "ui.h"
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

    Vector3D gravity = { 0.0f, physics->gravity * physics->dynamics.mass, 0.0f };
    DynamicsSystem::Integrate(physics->dynamics, deltaTime, physics->gravity, physics->onGround);



    transform->x = physics->dynamics.position.x;
    transform->y = physics->dynamics.position.y;

   
    physics->velX = physics->dynamics.velocity.x;
    physics->velY = physics->dynamics.velocity.y;

    // Ground collision
    if (transform->y <= physics->floorY) {
        transform->y = physics->dynamics.position.y = physics->floorY;
        physics->dynamics.velocity.y = 0.0f;
        physics->velY = 0.0f;
        physics->onGround = true;
    }
}

void PhysicsForces::jump(GameObject* object) {
    if (!object->hasComponent<Physics>() || !object->hasComponent<Transform>())
        return;

    Physics* physics = object->getComponent<Physics>();

    if (physics->onGround) {
        // Apply upward 
        Vector3D jumpDir = { 0.0f, 1.0f, 0.0f };
        DynamicsSystem::AddForce(physics->dynamics, jumpDir, physics->jumpForce, 0.1f);
        //physics->dynamics.velocity.y = 5.0f; // Instant upward velocity
        physics->onGround = false;
        //physics->canMove = true;
        UI::addDebugMessage("Player jumped!");
    }
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

    // Set bullet velocity to move horizontally along x-axis
    p->dynamics.velocity.x = 10.0f; // speed along x
    p->dynamics.velocity.y = 0.0f;  // no vertical motion

    // Optional: clear forces so bullet is not affected by gravity
    p->dynamics.forces.clear();

    UI::addDebugMessage("Bullet fired horizontally!");
}


