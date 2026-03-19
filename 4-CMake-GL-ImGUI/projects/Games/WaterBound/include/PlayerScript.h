#pragma once
 
#include "Component.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "mathlib.h"

struct PlayerScript {
    static void jump(GameObject* obj);

    static void shoot(GameObject* bullet, const Transform* origin, const Vector2D& targetPos);

    static GameObject* findAvailableBullet(GameObjectManager& manager);

    static void deactivateBullet(GameObject* bullet);

    static void createBulletPool(GameObjectManager& manager, const std::string& name = "bullet", int poolSize = 20);
};