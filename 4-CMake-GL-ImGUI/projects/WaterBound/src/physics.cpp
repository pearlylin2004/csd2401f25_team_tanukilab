#include "Physics.h"
#include "input.h"
#include <algorithm>


Vector2D playerPos = Vec_Set(-7.0f, 50.0f);
Vector2D playerVel = Vec_Zero();
Vector2D gravity = Vec_Set(0.0f, -980.0f); //////
bool isJumping = false;
float jumpPower = 600.0f;
float jumpSpeed = 300.0f;
float groundY = 50.0f;

// Memory pool for water 
std::vector<WaterDrop> PhysicsSystem::waterPool(PhysicsSystem::MAX_WATER_DROPS);

void PhysicsSystem::Init() {
    // Initialize all water drops as not alive
    for (auto& w : waterPool) {
        w.alive = false;
    }
}

void PhysicsSystem::SpawnWater(const Vector2D& startPos, const Vector2D& direction, float speed) {
    // Spawn multiple particle for a more fluid effect
    for (int i = 0; i < 8; i++) {
        // Find first available (dead) particle in pool
        auto it = std::find_if(waterPool.begin(), waterPool.end(),
            [](const WaterDrop& w) { return !w.alive; });

        if (it != waterPool.end()) {
            // Reuse this particle
            it->pos = startPos;
            Vector2D randomDir = direction;
            float angleVariation = 0.08f;
            float randomAngle = (float(rand()) / RAND_MAX - 0.8f) * angleVariation;
            it->vel = Vec_Rotate(&randomDir, randomAngle);
            it->vel = Vec_Scale(&it->vel, speed * (0.8f + 0.4f * float(rand()) / RAND_MAX));

            it->alive = true;
            it->lifeTime = 0.0f;
            it->maxLifeTime = 1.5f + (float(rand()) / RAND_MAX) * 0.5f;
            it->scale = 0.3f + (float(rand()) / RAND_MAX) * 0.4f;
        }
        // If pool is full, no particle created
    }
}

void PhysicsSystem::Update(float dt) {
    for (auto& w : waterPool) {
        if (!w.alive) continue;

        // Apply gravity for arcing motion
        Vector2D gravityStep = Vec_Scale(&gravity, 0.3f * dt);
        w.vel = Vec_Add(&w.vel, &gravityStep);

        // Update position
        Vector2D moveStep = Vec_Scale(&w.vel, dt);
        w.pos = Vec_Add(&w.pos, &moveStep);


        w.lifeTime += dt;

        //  dead when lifetime ends
        if (w.lifeTime > w.maxLifeTime) {
            w.alive = false;
        }

        // Check boundaries --> collision neded here
        /*if (w.pos.x < -2000 || w.pos.x > 2000 || w.pos.y < -2000 || w.pos.y > 2000) {
            w.alive = false;
        }*/

        // Check if hit the ground // just to check --> collision needed here
        if (w.pos.y <= groundY) {
            w.pos.y = groundY;
            w.vel = Vec_Scale(&w.vel, 0.3f);
        }
    }
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