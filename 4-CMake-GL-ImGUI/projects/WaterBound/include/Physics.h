#pragma once
#include "mathlib.h"
#include <vector>

// Declare variables as extern
extern Vector2D playerPos;
extern Vector2D playerVel;
extern Vector2D gravity;
extern bool isJumping;
extern float jumpPower;
extern float jumpSpeed;
extern float groundY;

struct WaterDrop {
    Vector2D pos;
    Vector2D vel;
    bool alive;
    float lifeTime;
    float maxLifeTime;
    float scale;
};

namespace PhysicsSystem {
    void Init();
    void Update(float dt);
    void SpawnWater(const Vector2D& startPos, const Vector2D& direction, float speed);

    // Memory pool variables
    extern std::vector<WaterDrop> waterPool;
    const size_t MAX_WATER_DROPS = 1000;
}

void UpdateJump(int direction, float dt);
Vector2D GetMouseWorldPosition();