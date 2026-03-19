
#include "WeaponSystem.h"
#include "Component.h"
#include "GameObject.h"
#include "input.h"
#include "mathlib.h"
#include "Editor/editorManager.h"
#include "performance.h"
#include <vector>
#include <cmath>

void WeaponSystem::update(GameObjectManager& manager, const float& deltaTime)
{
	auto start = std::chrono::high_resolution_clock::now();

	(void)deltaTime;

	#ifdef _DEBUG
		if (EditorManager::isEditingMode()) return;
	#endif

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	const Vector2D mouseWorld = InputHandler::getWorldMousePosition();

	for (GameObject* obj : gameObjects)
	{
		if (!obj->hasComponent<Transform>() || !obj->hasComponent<WeaponComponent>())
			continue;

		WeaponComponent* weapon = obj->getComponent<WeaponComponent>();
		if (!weapon->aimAtMouse) continue;

		Transform* weaponTransform = obj->getComponent<Transform>();

		//original local X offset
		//owner flips direction
		if (!weapon->baseLocalOffsetInitialised)
		{
			weapon->baseLocalOffsetX = weaponTransform->localOffset.x;
			weapon->baseLocalOffsetInitialised = true;
		}

		bool ownerFacingLeft = false;

		if (!weaponTransform->parentName.empty())
		{
			GameObject* owner = manager.getGameObject(weaponTransform->parentName);
			if (owner && owner->hasComponent<Transform>())
			{
				ownerFacingLeft = owner->getComponent<Transform>()->flipX;
			}
		}

		weaponTransform->localOffset.x = ownerFacingLeft
			? -std::abs(weapon->baseLocalOffsetX)
			: std::abs(weapon->baseLocalOffsetX);
		// Check the weapon's parent 
		if (!weaponTransform->parentName.empty())
		{
			GameObject* owner = manager.getGameObject(weaponTransform->parentName);
			if (owner && owner->hasComponent<Transform>())
			{
				Transform* pt = owner->getComponent<Transform>();
				const float parentRad = DegToRad(pt->rotation);
				const float cosA = cosf(parentRad);
				const float sinA = sinf(parentRad);
				// Rotate local offset around the parent
				const float rotatedX = weaponTransform->localOffset.x * cosA
					- weaponTransform->localOffset.y * sinA;
				const float rotatedY = weaponTransform->localOffset.x * sinA
					+ weaponTransform->localOffset.y * cosA;
				weaponTransform->x = pt->x + rotatedX;
				weaponTransform->y = pt->y + rotatedY;
			}
		}

		const float dx = mouseWorld.x - weaponTransform->x;
		const float dy = mouseWorld.y - weaponTransform->y;

		// Calculate the aiming angle
		const float aimAngleRad = ownerFacingLeft
			? std::atan2(-dy, -dx)  
			: std::atan2(dy, dx); 
		weaponTransform->flipX = ownerFacingLeft;

		const float aimAngleDeg = RadToDeg(aimAngleRad);
		weaponTransform->rotation = aimAngleDeg;

		weapon->currentAngle = aimAngleDeg;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count();
	g_SystemTimers.push_back({ "Weapon", ms });
}