#pragma once
 
#include "GameScript.h"
//#include "LogicContainer.h"
//#include "Systems.h"
//#include "Buoyancy.h"
//#include "PlayerScript.h"
//#include "WeaponSystem.h"

class LuaSystem;

class GameApp : public GameScript {
public:
    GameApp()  = default;
    ~GameApp() override = default;
 
    void onSceneLoad(GameObjectManager& manager) override;
    //void onSceneUnload(GameObjectManager& manager) override;
 
    // player fsm
    void update (GameObject& obj, float dt) override;
 
    // environmental FSMs
    void updateEnvironment(GameObject& obj, float dt) override;
 
    void updateAll(GameObjectManager& manager, float dt) override;

    void onPlayerInput(GameObject& obj, GameObjectManager& manager, MessageBus& messageBus, float dt) override;
 
    //void onPhysicsUpdate(GameObjectManager& manager, float dt) override;

    bool onCollision(GameObject* a, GameObject* b, CollisionInfo& info, GameObjectManager& manager) override;
    bool onCollisionResponse(GameObject* obj, CollisionInfo* col, CollisionInfo& info, bool isStatic, CollisionInfo* otherCol) override;

    Vector2D getAudioListenerPos(GameObjectManager& manager) override;

    void setLuaSystem(LuaSystem* lua) { m_luaSystem = lua; }

    //void registerLuaBindings(LuaSystem& lua) override;

    //// Direct access for CoreEngine to call WaterSystem::render()
    //WaterSystem& getWaterSystem() { return m_waterSystem; }
 
private:
    /*LogicContainer m_container;
    WaterSystem    m_waterSystem;
    BuoyancySystem m_buoyancy;
    WeaponSystem m_weaponSystem;

    GameObjectManager* m_manager = nullptr;*/

    LuaSystem* m_luaSystem = nullptr;
};

static void deactivateBullet(GameObject* bullet);