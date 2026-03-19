
#include "PlayerScript.h"

/**
 * @brief Applies jump force to make an object jump
 *
 * Applies an upward force to the object's dynamics system
 */
void PlayerScript::jump(GameObject* object) {
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
    //DebugLog::addMessage("Player jumped!", DebugMode::PlaySimul);
}

/**
 * @brief Initializes and launches a bullet from origin (player)
 *
 * Positions bullet at origin, sets velocity based on origin direction,
 * and configures bullet properties for horizontal movement
 */
void PlayerScript::shoot(GameObject* bullet, const Transform* origin, const Vector2D& targetPos) {
    if (!bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

	//reactivate bullet if it was previously used
    if (bullet->hasComponent<Render>()) {
        bullet->getComponent<Render>()->clr.a = 1.0f;
    }

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

    //float bulletSpeed = p->moveSpeed * 2.0f;  // Adjust multiplier as needed
    float bulletSpeed = p->moveSpeed * 3.0f;
    p->dynamics.velocity.x = dirX * bulletSpeed;
    p->dynamics.velocity.y = dirY * bulletSpeed;

    // Update flipX based on direction
    t->flipX = (dx < 0.0f);

    // Reset timers
    p->lifeTimer = 0.0f;

    //reset scale on shoot
    t->scaleX = p->initialScale;
    t->scaleY = p->initialScale;

    //p->gravityDelayTimer = 0;  // Initialize gravity immunity
    p->gravityDelayTimer = p->gravityDelay;
    p->alive = true;
	p->onGround = false;

    DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
}

/**
 * @brief Finds an available bullet from the object pool
 *
 * Searches through all game objects to find inactive bullets
 * following the naming pattern "bullet" followed by numbers.
 */

GameObject* PlayerScript::findAvailableBullet(GameObjectManager& manager) {

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
void PlayerScript::deactivateBullet(GameObject* bullet) {

    if (!bullet || !bullet->hasComponent<Physics>() || !bullet->hasComponent<Transform>())
        return;

    Physics* p = bullet->getComponent<Physics>();
    Transform* t = bullet->getComponent<Transform>();

    // Store original state when first deactivated
    if (!p->isOriginalStateSet) {
        p->originalPos = { t->x, t->y };
        //p->originalVel = { p->dynamics.velocity.x, p->dynamics.velocity.y };
        p->isOriginalStateSet = true;
    }

    // Reset to original state instead of hardcoded values
    t->x = p->originalPos.x;
    t->y = p->originalPos.y;

    p->dynamics.position.x = p->originalPos.x;
    p->dynamics.position.y = p->originalPos.y;
    p->dynamics.velocity.x = 0.0f;
    p->dynamics.velocity.y = 0.0f;

    p->dynamics.forces.clear();

    p->lifeTimer = 0.0f;
    p->alive = false;

	//transparent bullet when deactivated
    if (bullet->hasComponent<Render>()) {
        bullet->getComponent<Render>()->clr.a = 0.0f;  // make fully transparent
    }

    DebugLog::addMessage("Bullet deactivated and returned to pool", DebugMode::PlaySimul);
}

void PlayerScript::createBulletPool(GameObjectManager& manager, const std::string& templateBulletName, int poolSize) {
	GameObject* templateBullet = manager.getGameObject(templateBulletName);
	if (!templateBullet) {
		std::cerr << "Error: Template bullet '" << templateBulletName << "' not found!\n";
		return;
	}

	// ensure only 11 bullets (1+10 clone)
	if (templateBullet->getObjectName() != "bullet") manager.renameGameObject(templateBullet, "bullet");

	if (CollisionInfo* c = templateBullet->getComponent<CollisionInfo>()) {
		c->collisionTag = "bullet";
		c->ignoreCollisionsWith.insert("player");
		//c->ignoreCollisionsWith.insert("bullet");
	}
	std::cout << "Creating bullet pool with " << poolSize << " bullets...\n";

	for (int i = 1; i <= poolSize; ++i) {
		std::string bulletName = templateBulletName + std::to_string(i);

		if (manager.getGameObject(bulletName)) {
			std::cout << "  Bullet already exists: " << bulletName << "\n";
			continue;
		}

		GameObject* clonedBullet = manager.CloneGameObject(templateBulletName, bulletName);
		if (!clonedBullet) continue;

		clonedBullet->active = false;

		if (Transform* t = clonedBullet->getComponent<Transform>()) {
			t->x = -1000.0f;
			t->y = -1000.0f;
			t->z = 0.0f;
		}
		templateBullet->active = false; //deactivate the template bullet as well

		if (Physics* p = clonedBullet->getComponent<Physics>()) {
			p->alive = false;
			p->dynamics.velocity = Vector3D{ 0.0f, 0.0f, 0.0f };
		}

		if (CollisionInfo* c = clonedBullet->getComponent<CollisionInfo>()) {
			c->collisionTag = "bullet";
			c->ignoreCollisionsWith.insert("player");
			//c->ignoreCollisionsWith.insert("bullet");
		}
		/*GameObject* clonedBullet = *///CloneGameObject(templateBulletName, bulletName);
		//incase of bullets not cloned properly
		//if (clonedBullet) {
		//	if (clonedBullet->hasComponent<Transform>()) {
		//		Transform* t = clonedBullet->getComponent<Transform>();
		//		/*t->x = -1000.0f;
		//		t->y = -1000.0f;*/
		//	}
		//	if (clonedBullet->hasComponent<Physics>()) {
		//		Physics* p = clonedBullet->getComponent<Physics>();
		//		/*p->alive = false;
		//		p->dynamics.position.x = -1000.0f;
		//		p->dynamics.position.y = -1000.0f;
		//		p->dynamics.velocity.x = 0.0f;
		//		p->dynamics.velocity.y = 0.0f;
		//		p->dynamics.velocity.z = 0.0f;*/
		//	}
		// if (clonedBullet) {
		/*		if (CollisionInfo* c = clonedBullet->getComponent<CollisionInfo>()) {
					c->collisionTag = "bullet";
					c->ignoreCollisionsWith.insert("player");
					c->ignoreCollisionsWith.insert("bullet");
			}
		}*/
		//	std::cout << "  Created: " << bulletName << "\n";
		//}
	}
	std::cout << "Bullet pool creation completed.\n";
}