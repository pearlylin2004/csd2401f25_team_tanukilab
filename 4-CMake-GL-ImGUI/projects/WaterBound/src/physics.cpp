#include "physics.h"
#include "input.h"
#include "ui.h"

void PhysicsForces::applyGravity(Physics* physics, float deltaTime) {
    physics->velY += physics->gravity * deltaTime;
}

void PhysicsForces::applyDamping(Physics* physics) {
    physics->velX *= physics->damping;
}

void PhysicsForces::updatePosition(Transform* transform, Physics* physics, float deltaTime) {
    transform->x += physics->velX * deltaTime;
    transform->y += physics->velY * deltaTime;

    // Ground collision
    if (transform->y < physics->floorY) {
        transform->y = physics->floorY;
        physics->velY = 0.0f;
        physics->velX = 0.0f;
        physics->onGround = true;
    }
}

void PhysicsForces::jump(GameObject* object) {
    if (!object->hasComponent<Physics>() || !object->hasComponent<Transform>())
        return;

    Physics* physics = object->getComponent<Physics>();
    if (physics->onGround) {
        physics->velY = physics->jumpForce;
        physics->onGround = false;
        UI::addDebugMessage("Player jumped!");
    }
}

void PhysicsForces::shoot(GameObject* bullet, const Transform* origin) {
    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Transform* t = bullet->getComponent<Transform>();
    Physics* p = bullet->getComponent<Physics>();

    // Set bullet position to origin (player)
    t->x = origin->x;
    t->y = origin->y;
    t->z = 0.02f;
    t->scaleX = 0.5f;
    t->scaleY = 0.5f;
    t->scaleZ = 0.5f;
    t->rotation = 0.0f;

    // Set bullet velocity
    p->velX = 5.0f;
    p->velY = 5.0f;
    p->onGround = false;

    UI::addDebugMessage("Bullet spawned!");
}
