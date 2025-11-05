/* Start Header ************************************************************************/
/*!
\file		LogicContainer.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 30th, 2025
\brief      Defines a simple gameplay logic container used to handle per-object
            state machines (Idle, Walking, Jumping). This system bridges Input,
            Transform, and Physics components, allowing game objects such as
            players or NPCs to behave consistently each frame.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
class GameObject;
class GameObjectManager;
#include "Component.h"  
#include "input.h"      


class LogicContainer {
public:

    void update(GameObject& obj, float dt, GameObjectManager& manager);

private:
    static void enter(GameObject& obj, PlayerState next);
    static void tickIdle(GameObject& obj, float dt);
    static void tickWalking(GameObject& obj, float dt);
    static void tickJumping(GameObject& obj, float dt);
    static void tickFalling(GameObject& obj, float dt);

    static void trySpawnBulletNear(GameObject& obj, GameObjectManager& manager);
};

