/* Start Header ************************************************************************/
/*!
\file       LuaGameScript.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       March, 22th, 2026
\brief      This class acts as an abstract base class that defines the interface for 
            game logic scripts in the engine. It provides hooks for handling per-object
            updates, scene events, player input, collisions, physics, audio, and integration
            with Lua scripting. Derived classes (like LuaGameScript) implement these virtual
            methods to define concrete game behavior.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "mathlib.h"
 
class GameObject;
class GameObjectManager;
class MessageBus;
class LuaSystem;
struct CollisionInfo;

class GameScript {
public:
    virtual ~GameScript() = default;

    // @brief Optionally set LuaSystem if the script uses Lua.
    // @param lua Pointer to LuaSystem.
    virtual void setLuaSystem(LuaSystem* lua) { (void)lua; }

    // @brief Called when a scene is loaded.
    // @param manager Reference to the GameObjectManager.
    virtual void onSceneLoad(GameObjectManager& manager) { (void)manager; }

    // @brief Called when a scene is unloaded.
    // @param manager Reference to the GameObjectManager.
    virtual void onSceneUnload(GameObjectManager& manager) { (void)manager; }

    // @brief Update an individual GameObject.
    // @param obj Reference to the GameObject.
    // @param dt Delta time since last frame.
    virtual void update(GameObject& obj, float dt) = 0;

    // @brief Update environment-specific logic for a GameObject.
    // @param obj Reference to the GameObject.
    // @param dt Delta time since last frame.
    virtual void updateEnvironment(GameObject& obj, float dt) = 0;

    // @brief Update all objects managed by the manager.
    // @param manager Reference to GameObjectManager.
    // @param dt Delta time since last frame.
    virtual void updateAll(GameObjectManager& manager, float dt) = 0;

    // @brief Handle player input per object.
    // @param obj Reference to the GameObject.
    // @param manager Reference to GameObjectManager.
    // @param messageBus Reference to MessageBus for event dispatching.
    // @param dt Delta time since last frame.
    virtual void onPlayerInput(GameObject& obj, GameObjectManager& manager, MessageBus& messageBus, float dt) {
        (void)obj; (void)manager; (void)messageBus; (void)dt;
    }

    // @brief Physics update per manager (optional override).
    virtual void onPhysicsUpdate(GameObjectManager& manager, float dt) { (void)manager; (void)dt; }

    // @brief Handle collisions. Return true if the game handles it, false if engine should handle.
    virtual bool onCollision(GameObject* a, GameObject* b, CollisionInfo& info, GameObjectManager& manager) {
        (void)a; (void)b; (void)info; (void)manager;
        return false;
    }

    // @brief Called after engine collision handling; can override velocities.
    virtual bool onCollisionResponse(GameObject*, CollisionInfo*, CollisionInfo&, bool, CollisionInfo*) {
        return false;
    }

    // @brief Return the 2D position of the audio listener (usually player).
    virtual Vector2D getAudioListenerPos(GameObjectManager& manager) {
        (void)manager;
        return { 0.f, 0.f };
    }

    // @brief Register Lua bindings if needed.
    virtual void registerLuaBindings(class LuaSystem& lua) { (void)lua; }
};