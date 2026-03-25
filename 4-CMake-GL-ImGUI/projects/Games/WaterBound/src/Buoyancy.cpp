/* Start Header ************************************************************************/
/*!
\file        WaterSystem.cpp
\author     Asha Mathyalakan, asha.m, 2402886
             -100%
\par        asha.m@digipen.edu
\date        January, 28, 2026
\brief       Implementation of water system for animated water bodies, collision,
            buoyancy for object

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "Buoyancy.h"
#include "Component.h"
#include "GameObject.h"
#include "Editor/editorManager.h"
#include <vector>

void BuoyancySystem::update(GameObjectManager& manager, float deltaTime) {
    //std::vector<GameObject*> allObjects;
    //manager.getAllGameObjects(allObjects);

    //std::vector<GameObject*> waterBodies;
    //for (GameObject* obj : allObjects) {
    //    if (obj->hasComponent<WaterBody>()) {
    //        waterBodies.push_back(obj);
    //    }
    //}

    //for (GameObject* obj : allObjects) {
    //    Physics* physics = obj->getComponent<Physics>();
    //    Transform* transform = obj->getComponent<Transform>();

    //    if (!physics || !transform) continue;
    //    if (obj->hasComponent<WaterBody>()) continue;

    //    
    //    CollisionInfo* col = obj->getComponent<CollisionInfo>();
    //    bool isFloatingObject = col && (col->collisionTag == "floatingObject");
    //    if (EditorManager::isEditingMode() || EditorManager::isPaused()) {
    //        physics->velY = 0.f;
    //        physics->velX = 0.f;
    //        physics->dynamics.velocity.y = 0.f;
    //        physics->dynamics.velocity.x = 0.f;

    //        continue;
    //    }

    //    physics->inWater = false;


    //    if (isFloatingObject) {
    //        for (GameObject* waterObj : waterBodies) {
    //            WaterBody* water = waterObj->getComponent<WaterBody>();
    //            Transform* waterTransform = waterObj->getComponent<Transform>();
    //            if (!water || !waterTransform) continue;


    //            float waterHalfW = water->width * 0.5f;
    //            float waterHalfH = water->height * 0.5f;
    //            float waterMinX = waterTransform->x - waterHalfW;
    //            float waterMaxX = waterTransform->x + waterHalfW;
    //            float waterMinY = waterTransform->y - waterHalfH;
    //            float waterMaxY = waterTransform->y + waterHalfH;

    //            bool insideWater = (transform->x >= waterMinX && transform->x <= waterMaxX &&
    //                transform->y >= waterMinY && transform->y <= waterMaxY);

    //            if (!insideWater && !physics->inWater) continue;

    //            physics->inWater = true;

    //            float waterSurfaceY = waterTransform->y + waterHalfH;

    //            if (water->lastFrameRise > 0.0f) {
    //                transform->y += water->lastFrameRise;
    //                physics->velY = 0.0f;
    //                physics->dynamics.position.y = transform->y;
    //                break;
    //            }

    //            float depth = waterSurfaceY - transform->y;
    //            float halfObjHeight = transform->scaleY * 0.5f;
    //            float submergedFraction = glm::clamp(depth / (halfObjHeight * 2.0f + 0.001f), 0.0f, 1.0f);

    //            float safeMass = physics->dynamics.mass;
    //            float buoyancyAccel = (water->buoyancyForce / safeMass) * submergedFraction * 0.016f * 16.0f;

    //            physics->velY += buoyancyAccel;
    //            physics->velY = glm::clamp(physics->velY, physics->maxFallSpeed, 8.0f);

    //            /*float strongDrag = 0.999f;*/
    //            float verticalDrag = glm::mix(water->waterDrag * 0.999f, water->waterDrag, submergedFraction);
    //            physics->velY *= verticalDrag;
    //            physics->velX *= water->waterDrag;

    //            break;
    //        }
    //    }
    //}
}