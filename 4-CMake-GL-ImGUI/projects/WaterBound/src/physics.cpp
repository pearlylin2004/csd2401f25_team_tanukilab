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



void UpdateJump(int direction, float dt)
{
    if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE) && !isJumping)
    {
        isJumping = true;
        playerVel = Vec_Set(direction * jumpSpeed, jumpPower);
    }

    if (isJumping)
    {
        // Apply gravity: vel = vel + g * dt
        Vector2D gStep = Vec_Scale(&gravity, dt);
        playerVel = Vec_Add(&playerVel, &gStep);

        // Update position: pos = pos + vel * dt
        Vector2D moveStep = Vec_Scale(&playerVel, dt);
        playerPos = Vec_Add(&playerPos, &moveStep);

        // Check for ground collision
        if (playerPos.y <= groundY)
        {
            playerPos.y = groundY;
            playerVel = Vec_Zero();
            isJumping = false;
        }
    }
}

//Vector2D GetMouseWorldPosition() {
//    Vector2D mouse;
//    mouse = InputHandler::getCursorPosition();
//
//    float worldX = (float)mouse.x - 800.0f;
//    float worldY = (float)(900 - mouse.y) - 450.0f;
//
//    return Vec_Set(worldX, worldY);
//}
