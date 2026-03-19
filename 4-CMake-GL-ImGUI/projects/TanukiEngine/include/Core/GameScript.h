
#pragma once
#include "mathlib.h"
 
class GameObject;
class GameObjectManager;
class MessageBus;
struct CollisionInfo;

class GameScript {
public:
    virtual ~GameScript() = default;

    virtual void onSceneLoad(GameObjectManager& manager) { (void)manager; }
    virtual void onSceneUnload(GameObjectManager& manager) { (void)manager; }

    virtual void update(GameObject& obj, float dt) = 0;

    virtual void updateEnvironment(GameObject& obj, float dt) = 0;

    virtual void updateAll(GameObjectManager& manager, float dt) = 0;

    virtual void onPlayerInput(GameObject& obj, GameObjectManager& manager, MessageBus& messageBus, float dt) { (void)obj; (void)manager; (void)messageBus; (void)dt; }

    virtual void onPhysicsUpdate(GameObjectManager& manager, float dt) { (void)manager; (void)dt; }

    // true then game handle, else handle by engine
    virtual bool onCollision(GameObject* a, GameObject* b, CollisionInfo& info, GameObjectManager& manager) { 
        (void)a; (void)b; (void)info; (void)manager;
        return false; 
    }

    // Called after engine, game can override velocities
    virtual bool onCollisionResponse(GameObject*, CollisionInfo*, CollisionInfo&, bool, CollisionInfo*) { return false; }

    // for audio 3d listener (usually is player)
    virtual Vector2D getAudioListenerPos(GameObjectManager& manager) { 
        (void)manager; 
        return { 0.f, 0.f };
    }
};