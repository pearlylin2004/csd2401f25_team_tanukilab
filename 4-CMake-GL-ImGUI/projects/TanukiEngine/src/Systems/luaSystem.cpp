/* Start Header ************************************************************************/
/*!
\file       lua.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Ocotber, 23rd, 2025
\brief

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "luaSystem.h"
#include "Systems.h"

void LuaSystem::init(GameObjectManager& manager) {
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_register(L, "Input_isKeyHeld", Lua_IsKeyHeld);
    lua_register(L, "getPosition", Lua_getPosition);
    lua_register(L, "setPosition", Lua_setPosition);

    //for message
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, Lua_SendInputEvent, 1);
    lua_setglobal(L, "SendInputEvent");
    //manager = std::make_unique<GameObjectManager>();

    // Render
    lua_register(L, "setRenderAlpha", Lua_setRenderAlpha);
    lua_register(L, "setTexture", Lua_setTexture);
    lua_register(L, "setColliderSize", Lua_setColliderSize);

    // physics getters
    lua_register(L, "getVelX", Lua_getVelX);
    lua_register(L, "getVelY", Lua_getVelY);
    lua_register(L, "isOnGround", Lua_isOnGround);
    lua_register(L, "getCanMove", Lua_getCanMove);
    lua_register(L, "getMoveSpeed", Lua_getMoveSpeed);
    lua_register(L, "getJumpForce", Lua_getJumpForce);
    lua_register(L, "isInWater", Lua_isInWater);
    lua_register(L, "getGravity", Lua_getGravity);
    lua_register(L, "canShoot", Lua_canShoot);
    lua_register(L, "getShootTimer", Lua_getShootTimer);
    lua_register(L, "isAlive", Lua_isAlive);
    lua_register(L, "getFireRate", Lua_getFireRate);
    lua_register(L, "getLifeTimer", Lua_getLifeTimer);
    lua_register(L, "getMaxLifetime", Lua_getMaxLifetime);
    lua_register(L, "getGravityDelayTimer", Lua_getGravityDelayTimer);
    lua_register(L, "getGravityDelay", Lua_getGravityDelay);
    lua_register(L, "getInitialScale", Lua_getInitialScale);
    lua_register(L, "getMinScale", Lua_getMinScale);
    lua_register(L, "getDynVelX", Lua_getDynVelX);
    lua_register(L, "getDynVelY", Lua_getDynVelY);
    lua_register(L, "getDynPosX", Lua_getDynPosX);
    lua_register(L, "getDynPosY", Lua_getDynPosY);
    lua_register(L, "getDynMass", Lua_getDynMass);
    lua_register(L, "getCoyoteTime", Lua_getCoyoteTime);
    lua_register(L, "getMaxFallSpeed", Lua_getMaxFallSpeed);
    lua_register(L, "getSinlePersonalBool", Lua_getSinlePersonalBool);
    lua_register(L, "getSinleFlag", Lua_getSinleFlag);
    lua_register(L, "getHitground", Lua_getHitground);
    lua_register(L, "getSinleGravity", Lua_getSinleGravity);
    lua_register(L, "getWaterBounce", Lua_getWaterBounce);
    lua_register(L, "getAccelX", Lua_getAccelX);
    lua_register(L, "getAccelY", Lua_getAccelY);


    // physics setters
    lua_register(L, "setVelX", Lua_setVelX);
    lua_register(L, "setVelY", Lua_setVelY);
    lua_register(L, "setOnGround", Lua_setOnGround);
    lua_register(L, "setCanMove", Lua_setCanMove);
    lua_register(L, "setGravity", Lua_setGravity);
    lua_register(L, "setShootTimer", Lua_setShootTimer);
    lua_register(L, "setPhysicsFlag", Lua_setPhysicsFlag);
    lua_register(L, "applyDamping", Lua_applyDamping);
    lua_register(L, "updatePosition", Lua_updatePosition);
    lua_register(L, "setAlive", Lua_setAlive);
    lua_register(L, "setLifeTimer", Lua_setLifeTimer);
    lua_register(L, "setGravityDelayTimer", Lua_setGravityDelayTimer);
    lua_register(L, "setDynVelX", Lua_setDynVelX);
    lua_register(L, "setDynVelY", Lua_setDynVelY);
    lua_register(L, "setDynPos", Lua_setDynPos);
    lua_register(L, "updateForces", Lua_updateForces);
    lua_register(L, "advanceDynamics", Lua_advanceDynamics);
    lua_register(L, "setInWater", Lua_setInWater);
    lua_register(L, "clearDynForces", Lua_clearDynForces);
    lua_register(L, "setOriginalPos", Lua_setOriginalPos);
    lua_register(L, "isOriginalStateSet", Lua_isOriginalStateSet);
    lua_register(L, "setAutoUpdate", Lua_setAutoUpdate);
    lua_register(L, "setAccelX", Lua_setAccelX);
    lua_register(L, "setAccelY", Lua_setAccelY);
    lua_register(L, "setHitground", Lua_setHitground);


    // collision
    lua_register(L, "getCollisionFlag", Lua_getCollisionFlag);
    lua_register(L, "setCollisionFlag", Lua_setCollisionFlag);
    lua_register(L, "setCollisionTag", Lua_setCollisionTag);
    lua_register(L, "getFriction", Lua_getFriction);


    // transform
    lua_register(L, "getRotation", Lua_getRotation);
    lua_register(L, "setRotation", Lua_setRotation);
    lua_register(L, "setScale", Lua_setScale);
    lua_register(L, "getScaleX", Lua_getScaleX);
    lua_register(L, "getScaleY", Lua_getScaleY);
    lua_register(L, "getLocalOffsetX", Lua_getLocalOffsetX);
    lua_register(L, "setLocalOffsetX", Lua_setLocalOffsetX);
    lua_register(L, "setFlipX", Lua_setFlipX);
    lua_register(L, "getFlipX", Lua_getFlipX);

    // input bindings
    lua_register(L, "isKeyTriggered", Lua_isKeyTriggered);
    lua_register(L, "isKeyReleased", Lua_isKeyReleased);
    lua_register(L, "isMouseLeftHeld", Lua_isMouseLeftHeld);
    lua_register(L, "isMouseRightHeld", Lua_isMouseRightHeld);
    lua_register(L, "isMouseLeftTriggered", Lua_isMouseLeftTriggered);
    lua_register(L, "getMouseWorldX", Lua_getMouseWorldX);
    lua_register(L, "getMouseWorldY", Lua_getMouseWorldY);
    lua_register(L, "getEditorMouseWorldX", Lua_getEditorMouseWorldX);
    lua_register(L, "getEditorMouseWorldY", Lua_getEditorMouseWorldY);

    // FSM
    lua_register(L, "getPlayerState", Lua_getPlayerState);
    lua_register(L, "setPlayerState", Lua_setPlayerState);
    lua_register(L, "isFixState", Lua_isFixState);
    lua_register(L, "setFixState", Lua_setFixState);
    lua_register(L, "getFacingRight", Lua_getFacingRight);
    lua_register(L, "setFacingRight", Lua_setFacingRight);
    lua_register(L, "getStateTime", Lua_getStateTime);
    lua_register(L, "setStateTime", Lua_setStateTime);

    // Audio
    lua_register(L, "playSound", Lua_playSound);
    lua_register(L, "stopSound", Lua_stopSound);
    lua_register(L, "setSoundVolume", Lua_setSoundVolume);
    lua_register(L, "isSoundPlaying", Lua_isSoundPlaying);

    // visual script
    lua_register(L, "getVSGlobalFloat", Lua_getVSGlobalFloat);
    lua_register(L, "setVSGlobalFloat", Lua_setVSGlobalFloat);
    lua_register(L, "hasVSGlobalFloat", Lua_hasVSGlobalFloat);
    lua_register(L, "getNumCurCollisions", Lua_getNumCurCollisions);
    lua_register(L, "getCurCollision", Lua_getCurCollision);

    // Weapon
    lua_register(L, "getAimAtMouse", Lua_getAimAtMouse);
    lua_register(L, "getWeaponBaseOffsetX", Lua_getWeaponBaseOffsetX);
    lua_register(L, "setWeaponBaseOffsetX", Lua_setWeaponBaseOffsetX);
    lua_register(L, "getLocalOffsetY", Lua_getLocalOffsetY);
    lua_register(L, "isWeaponOffsetInitialised", Lua_isWeaponOffsetInitialised);
    lua_register(L, "setWeaponOffsetInitialised", Lua_setWeaponOffsetInitialised);
    lua_register(L, "getWeaponCurrentAngle", Lua_getWeaponCurrentAngle);
    lua_register(L, "setWeaponCurrentAngle", Lua_setWeaponCurrentAngle);

    // WaterBody
    lua_register(L, "hasWaterBody", Lua_hasWaterBody);
    lua_register(L, "getWaterWidth", Lua_getWaterWidth);
    lua_register(L, "getWaterHeight", Lua_getWaterHeight);
    lua_register(L, "getWaterBuoyancyForce", Lua_getWaterBuoyancyForce);
    lua_register(L, "getWaterDrag", Lua_getWaterDrag);
    lua_register(L, "getWaterLastFrameRise", Lua_getWaterLastFrameRise);
    lua_register(L, "addWaterHeight", Lua_addWaterHeight);

    // ledge climb
    lua_register(L, "getLedgeClimb", Lua_getLedgeClimb);
    lua_register(L, "setLedgeClimb", Lua_setLedgeClimb);
    lua_register(L, "getLedgeTargetY", Lua_getLedgeTargetY);
    lua_register(L, "getLedgeClimbSpeed", Lua_getLedgeClimbSpeed);
    lua_register(L, "getJumpBufferTime", Lua_getJumpBufferTime);

    // Editor State
    lua_register(L, "isEditingMode", Lua_isEditingMode);
    lua_register(L, "isPaused", Lua_isPaused);
    lua_register(L, "isEditorOpen", Lua_isEditorOpen);
    lua_register(L, "isSceneHovered", Lua_isSceneHovered);

    // debug
    lua_register(L, "debugLog", Lua_debugLog);
    lua_register(L, "debugLogPlay", Lua_debugLogPlay);

    registerManagerBindings(manager);
}

void LuaSystem::cleanup() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

//function exposed to Lua
int LuaSystem::Lua_IsKeyHeld(lua_State* L) {
    const char* keyStr = lua_tostring(L, 1); //get string from Lua
    if (!keyStr) { lua_pushboolean(L, 0); return 1; }

    //convert string to GLFW key code
    int key = -1;
    if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "SPACE") == 0) key = GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "B") == 0) key = GLFW_KEY_B;
    else if (strcmp(keyStr, "E") == 0) key = GLFW_KEY_E;
    else if (strcmp(keyStr, "R") == 0) key = GLFW_KEY_R;
    //add more keys if needed in the future

    bool held = false;
    if (key != -1) {
        held = InputHandler::isKeyHeld(key);
    }

    lua_pushboolean(L, held); //push result to Lua
    return 1; //number of return values
}

bool LuaSystem::runString(const std::string& code) {
    if (!L) return false;
    return luaL_dostring(L, code.c_str()) == LUA_OK;
}

bool LuaSystem::runScript(const std::string& filename) {
    if (!L) return false;
    return luaL_dofile(L, filename.c_str()) == LUA_OK;
}

float LuaSystem::getGlobalNumber(const std::string& name) {
    if (!L) return 0.0f;
    lua_getglobal(L, name.c_str());
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        return 0.0f;
    }
    float val = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);
    return val;
}

//get position of object from Lua
int LuaSystem::Lua_getPosition(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj) return 0;

    //get the object Transform component
    Transform* transform = obj->getComponent<Transform>();
    if (!transform)
        return 0;

    lua_pushnumber(L, transform->x);
    lua_pushnumber(L, transform->y);
    //lua_pushnumber(L, transform->z); //not needed for 2D
    return 2; //return x and y
}

//set position of object from Lua
int LuaSystem::Lua_setPosition(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj)
        return 0;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform)
        return 0;

    float x = static_cast<float>(lua_tonumber(L, 2));
    float y = static_cast<float>(lua_tonumber(L, 3));
    //float z = static_cast<float>(lua_tonumber(L, 4)); //idk if needed cause in the transform i see there is z

    transform->x = x;
    transform->y = y;
    //transform->z = z; //idk if needed cause in the transform i see there is z

    return 0;
}

//for message
int LuaSystem::Lua_SendInputEvent(lua_State* L) {
    LuaSystem* self = static_cast<LuaSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!self || !self->messageBus) return 0;

    const char* key = lua_tostring(L, 1);
    float x = static_cast<float>(lua_tonumber(L, 2));
    float y = static_cast<float>(lua_tonumber(L, 3));

    //to set the keys 
    KeyEvent evt;
    evt.key = key ? key : "";
    evt.pressed = true;
    evt.x = x;
    evt.y = y;

    self->messageBus->publish(Message("KeyPressed", nullptr, evt));

    return 0;
}

/* ---- render ---- */
int LuaSystem::Lua_setRenderAlpha(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Render>()) return 0;

    obj->getComponent<Render>()->clr.a = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setTexture(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Render>()) return 0;

    const char* tex = lua_tostring(L, 2);

    if (!tex) return 0;

    Render* r = obj->getComponent<Render>();

    r->texFile = tex;
    r->texChanged = true;
    return 0;
}

int LuaSystem::Lua_setColliderSize(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) return 0;

    CollisionInfo* c = obj->getComponent<CollisionInfo>();

    c->colliderSize.x = static_cast<float>(lua_tonumber(L, 2));
    c->colliderSize.y = static_cast<float>(lua_tonumber(L, 3));

    return 0;
}


/* ---- physics getters ---- */
int LuaSystem::Lua_getVelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->velX);

    return 1;
}

int LuaSystem::Lua_getVelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->velY);

    return 1;
}

int LuaSystem::Lua_isOnGround(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->onGround ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getCanMove(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->canMove ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getMoveSpeed(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->moveSpeed);

    return 1;
}

int LuaSystem::Lua_getJumpForce(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->jumpForce);

    return 1;
}

int LuaSystem::Lua_isInWater(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->inWater ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getGravity(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->gravity);

    return 1;
}

int LuaSystem::Lua_canShoot(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->canShoot ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getShootTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->shootTimer);

    return 1;
}

int LuaSystem::Lua_getFireRate(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->fireRate);

    return 1;
}

int LuaSystem::Lua_isAlive(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->alive ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getLifeTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->lifeTimer);

    return 1;
}

int LuaSystem::Lua_getMaxLifetime(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->maxLifetime);

    return 1;
}

int LuaSystem::Lua_getGravityDelayTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->gravityDelayTimer);

    return 1;
}

int LuaSystem::Lua_getGravityDelay(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->gravityDelay);

    return 1;
}

int LuaSystem::Lua_getInitialScale(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 1); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->initialScale);

    return 1;
}

int LuaSystem::Lua_getMinScale(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->minScale);

    return 1;
}

int LuaSystem::Lua_getDynVelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->dynamics.velocity.x);

    return 1;
}

int LuaSystem::Lua_getDynVelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->dynamics.velocity.y);

    return 1;
}

int LuaSystem::Lua_getDynPosX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->dynamics.position.x);

    return 1;
}

int LuaSystem::Lua_getDynPosY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->dynamics.position.y);

    return 1;
}

int LuaSystem::Lua_getDynMass(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 1); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->dynamics.mass);

    return 1;
}

int LuaSystem::Lua_getCoyoteTime(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0.12); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->coyoteTime);

    return 1;
}

int LuaSystem::Lua_getMaxFallSpeed(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, -20); return 1; }
    lua_pushnumber(L, obj->getComponent<Physics>()->maxFallSpeed);
    return 1;
}

int LuaSystem::Lua_getSinlePersonalBool(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->sinlePersonalBool ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getSinleFlag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->sinleFlag ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getHitground(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->hitground ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_getSinleGravity(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->sinleGravity);
    return 1;
}

int LuaSystem::Lua_getWaterBounce(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->waterBounce);
    return 1;
}

int LuaSystem::Lua_getAccelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->acceleration.x);
    return 1;
}

int LuaSystem::Lua_getAccelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->acceleration.y);
    return 1;
}


/* ---- physics setters ---- */
int LuaSystem::Lua_setVelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    Physics* p = obj->getComponent<Physics>();

    p->velX = static_cast<float>(lua_tonumber(L, 2));
    p->dynamics.velocity.x = p->velX;

    return 0;
}

int LuaSystem::Lua_setVelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    Physics* p = obj->getComponent<Physics>();
    p->velY = static_cast<float>(lua_tonumber(L, 2));
    p->dynamics.velocity.y = p->velY;

    return 0;
}

int LuaSystem::Lua_setOnGround(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->onGround = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_setCanMove(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->canMove = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_setGravity(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->gravity = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setShootTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->shootTimer = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setPhysicsFlag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->physicsFlag = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_applyDamping(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    float friction = static_cast<float>(lua_tonumber(L, 2));
    float targetVelX = static_cast<float>(lua_tonumber(L, 3));
    float dt = static_cast<float>(lua_tonumber(L, 4));

    PhysicsForces::applyDamping(obj->getComponent<Physics>(), friction, targetVelX, dt);

    return 0;
}

int LuaSystem::Lua_updatePosition(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj) return 0;

    Transform* t = obj->getComponent<Transform>();
    Physics* p = obj->getComponent<Physics>();
    if (!t || !p) return 0;

    float dt = static_cast<float>(lua_tonumber(L, 2));

    PhysicsForces::updatePosition(t, p, dt);

    return 0;
}

int LuaSystem::Lua_setAlive(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->alive = lua_toboolean(L, 2) != 0;

    return 0;
}
int LuaSystem::Lua_setLifeTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->lifeTimer = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}
int LuaSystem::Lua_setGravityDelayTimer(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->gravityDelayTimer = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}
int LuaSystem::Lua_setDynVelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->dynamics.velocity.x = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setDynVelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->dynamics.velocity.y = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setDynPos(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;
    Physics* p = obj->getComponent<Physics>();

    p->dynamics.position.x = static_cast<float>(lua_tonumber(L, 2));
    p->dynamics.position.y = static_cast<float>(lua_tonumber(L, 3));

    return 0;
}

int LuaSystem::Lua_updateForces(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    float dt = static_cast<float>(lua_tonumber(L, 2));

    DynamicsSystem::UpdateForces(obj->getComponent<Physics>()->dynamics, dt);
    return 0;
}

int LuaSystem::Lua_advanceDynamics(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    Physics* p = obj->getComponent<Physics>();
    float dt = static_cast<float>(lua_tonumber(L, 2));
    Vector3D deltaPos = Vec3_Scale(&p->dynamics.velocity, dt);

    p->dynamics.position = Vec3_Add(&p->dynamics.position, &deltaPos);

    Transform* t = obj->getComponent<Transform>();
    if (t) { t->x = p->dynamics.position.x; t->y = p->dynamics.position.y; }

    return 0;
}

int LuaSystem::Lua_setAccelX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->acceleration.x = (float)lua_tonumber(L, 2);
    return 0;
}

int LuaSystem::Lua_setAccelY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->acceleration.y = (float)lua_tonumber(L, 2);
    return 0;
}

int LuaSystem::Lua_setHitground(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->hitground = lua_toboolean(L, 2) != 0;
    return 0;
}

/* ---- transform ---- */
int LuaSystem::Lua_getRotation(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Transform>()->rotation);
    return 1;
}

int LuaSystem::Lua_setRotation(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) return 0;

    obj->getComponent<Transform>()->rotation = static_cast<float>(lua_tonumber(L, 2));
    return 0;
}

int LuaSystem::Lua_setScale(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) return 0;

    Transform* t = obj->getComponent<Transform>();
    t->scaleX = static_cast<float>(lua_tonumber(L, 2));
    t->scaleY = static_cast<float>(lua_tonumber(L, 3));

    return 0;
}

int LuaSystem::Lua_getScaleX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushnumber(L, 1); return 1; }

    lua_pushnumber(L, obj->getComponent<Transform>()->scaleX);

    return 1;
}

int LuaSystem::Lua_getScaleY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushnumber(L, 1); return 1; }

    lua_pushnumber(L, obj->getComponent<Transform>()->scaleY);

    return 1;
}
int LuaSystem::Lua_getLocalOffsetX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Transform>()->localOffset.x);

    return 1;
}
int LuaSystem::Lua_setLocalOffsetX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) return 0;

    obj->getComponent<Transform>()->localOffset.x = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_setFlipX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) return 0;

    obj->getComponent<Transform>()->flipX = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_getFlipX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Transform>()->flipX ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setInWater(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->inWater = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_clearDynForces(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->dynamics.forces.clear();

    return 0;
}

int LuaSystem::Lua_setOriginalPos(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    Physics* p = obj->getComponent<Physics>();
    Transform* t = obj->getComponent<Transform>();

    if (!t) return 0;
    p->originalPos = { t->x, t->y };
    p->isOriginalStateSet = true;

    return 0;
}

int LuaSystem::Lua_isOriginalStateSet(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->isOriginalStateSet ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setAutoUpdate(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->autoUpdate = lua_toboolean(L, 2) != 0;

    return 0;
}

/* ---- collision ---- */
int LuaSystem::Lua_getCollisionFlag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<CollisionInfo>()->collisionFlag ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setCollisionFlag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) return 0;

    obj->getComponent<CollisionInfo>()->collisionFlag = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_setCollisionTag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) return 0;

    const char* tag = lua_tostring(L, 2);
    if (!tag) return 0;

    obj->getComponent<CollisionInfo>()->collisionTag = tag;

    return 0;
}

int LuaSystem::Lua_getFriction(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) { lua_pushnumber(L, 0); return 1; }
    lua_pushnumber(L, obj->getComponent<CollisionInfo>()->material.friction);
    return 1;
}


/* ---- input bindings ---- */
int LuaSystem::Lua_isKeyTriggered(lua_State* L) {
    const char* keyStr = lua_tostring(L, 1);
    if (!keyStr) { lua_pushboolean(L, 0); return 1; }

    int key = -1;
    if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "SPACE") == 0) key = GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "B") == 0) key = GLFW_KEY_B;
    else if (strcmp(keyStr, "E") == 0) key = GLFW_KEY_E;
    else if (strcmp(keyStr, "R") == 0) key = GLFW_KEY_R;

    lua_pushboolean(L, key != -1 && InputHandler::isKeyTriggered(key) ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isKeyReleased(lua_State* L) {
    const char* keyStr = lua_tostring(L, 1);
    if (!keyStr) { lua_pushboolean(L, 0); return 1; }
    int key = -1;
    if (strcmp(keyStr, "SPACE") == 0) key = GLFW_KEY_SPACE;
    else if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    lua_pushboolean(L, key != -1 && InputHandler::isKeyReleased(key) ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isMouseLeftHeld(lua_State* L) {
    lua_pushboolean(L, InputHandler::isMouseLeftHeld() ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isMouseRightHeld(lua_State* L) {
    lua_pushboolean(L, InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_RIGHT) ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isMouseLeftTriggered(lua_State* L) {
    lua_pushboolean(L, InputHandler::isMouseLeftClicked() ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_getMouseWorldX(lua_State* L) {
    lua_pushnumber(L, InputHandler::getWorldMousePosition().x);
    return 1;
}

int LuaSystem::Lua_getMouseWorldY(lua_State* L) {
    lua_pushnumber(L, InputHandler::getWorldMousePosition().y);
    return 1;
}

int LuaSystem::Lua_getEditorMouseWorldX(lua_State* L) {
#ifdef _DEBUG
    ImVec2 scenePos, sceneSize;
    EditorManager::getScenePosNSize(scenePos, sceneSize);
    lua_pushnumber(L, InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize).x);
#else
    lua_pushnumber(L, InputHandler::getWorldMousePosition().x);
#endif
    return 1;
}

int LuaSystem::Lua_getEditorMouseWorldY(lua_State* L) {
#ifdef _DEBUG
    ImVec2 scenePos, sceneSize;
    EditorManager::getScenePosNSize(scenePos, sceneSize);
    lua_pushnumber(L, InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize).y);
#else
    lua_pushnumber(L, InputHandler::getWorldMousePosition().y);
#endif
    return 1;
}


/* ---- FSM ---- */
int LuaSystem::Lua_getPlayerState(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) {
        lua_pushstring(L, ""); return 1;
    }

    StateMachine* sm = obj->getComponent<StateMachine>();

    switch (sm->state) {
        case PlayerState::Idle:     lua_pushstring(L, "Idle");     break;
        case PlayerState::Walking:  lua_pushstring(L, "Walking");  break;
        case PlayerState::Jumping:  lua_pushstring(L, "Jumping");  break;
        case PlayerState::Falling:  lua_pushstring(L, "Falling");  break;
        case PlayerState::Dead:     lua_pushstring(L, "Dead");     break;
        default:                    lua_pushstring(L, "Idle");     break;
    }

    return 1;
}

int LuaSystem::Lua_setPlayerState(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) return 0;

    const char* state = lua_tostring(L, 2);

    StateMachine* sm = obj->getComponent<StateMachine>();

    if (strcmp(state, "Idle") == 0) sm->state = PlayerState::Idle;
    else if (strcmp(state, "Walking") == 0) sm->state = PlayerState::Walking;
    else if (strcmp(state, "Jumping") == 0) sm->state = PlayerState::Jumping;
    else if (strcmp(state, "Falling") == 0) sm->state = PlayerState::Falling;
    else if (strcmp(state, "Dead") == 0) sm->state = PlayerState::Dead;

    return 0;
}

int LuaSystem::Lua_isFixState(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<StateMachine>()->fixState ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setFixState(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) return 0;

    obj->getComponent<StateMachine>()->fixState = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_getFacingRight(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) { lua_pushboolean(L, 1); return 1; }

    lua_pushboolean(L, obj->getComponent<StateMachine>()->facingRight ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setFacingRight(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) return 0;

    obj->getComponent<StateMachine>()->facingRight = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_getStateTime(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<StateMachine>()->stateTime);

    return 1;
}

int LuaSystem::Lua_setStateTime(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<StateMachine>()) return 0;

    obj->getComponent<StateMachine>()->stateTime = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}


/* ---- audio ---- */
int LuaSystem::Lua_playSound(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<AudioComponent>()) return 0;

    const char* channelName = lua_tostring(L, 2);
    if (!channelName) return 0;

    AudioChannel* ch = obj->getComponent<AudioComponent>()->getChannel(channelName);
    if (!ch) return 0;
    if (ch->audioFile.empty()) return 0;
    // dont play if alr playing
    if (ch->state == AudioState::Playing || ch->isPendingPlay) return 0;

    ch->isPendingPlay = true;
    return 0;
}

int LuaSystem::Lua_stopSound(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<AudioComponent>()) return 0;

    const char* channelName = lua_tostring(L, 2);
    if (!channelName) return 0;

    AudioChannel* ch = obj->getComponent<AudioComponent>()->getChannel(channelName);

    if (ch) ch->isPendingStop = true;

    return 0;
}

int LuaSystem::Lua_setSoundVolume(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<AudioComponent>()) return 0;

    const char* channelName = lua_tostring(L, 2);
    float volume = static_cast<float>(lua_tonumber(L, 3));

    if (!channelName) return 0;

    AudioChannel* ch = obj->getComponent<AudioComponent>()->getChannel(channelName);

    if (ch) ch->volume = volume;

    return 0;
}

int LuaSystem::Lua_isSoundPlaying(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<AudioComponent>()) {
        lua_pushboolean(L, 0); return 1;
    }

    const char* channelName = lua_tostring(L, 2);
    if (!channelName) { lua_pushboolean(L, 0); return 1; }

    AudioChannel* ch = obj->getComponent<AudioComponent>()->getChannel(channelName);

    lua_pushboolean(L, (ch && ch->state == AudioState::Playing) ? 1 : 0);

    return 1;
}


/* ---- visual script ---- */
int LuaSystem::Lua_getVSGlobalFloat(lua_State* L) {
    const char* key = lua_tostring(L, 1);
    if (!key) { lua_pushnumber(L, 0); return 1; }
    auto it = VisualLC::globalFloatVar.find(key);
    lua_pushnumber(L, it != VisualLC::globalFloatVar.end() ? it->second : 0.f);
    return 1;
}

int LuaSystem::Lua_setVSGlobalFloat(lua_State* L) {
    const char* key = lua_tostring(L, 1);
    if (!key) return 0;
    VisualLC::globalFloatVar[key] = static_cast<float>(lua_tonumber(L, 2));
    return 0;
}

int LuaSystem::Lua_hasVSGlobalFloat(lua_State* L) {
    const char* key = lua_tostring(L, 1);
    if (!key) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, VisualLC::globalFloatVar.count(key) ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getNumCurCollisions(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<VisualLC>()) { lua_pushinteger(L, 0); return 1; }

    lua_pushinteger(L, (int)obj->getComponent<VisualLC>()->curCollision.size());

    return 1;
}

int LuaSystem::Lua_getCurCollision(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    int idx = (int)lua_tointeger(L, 2) - 1; // 1-based
    if (!obj || !obj->hasComponent<VisualLC>()) { lua_pushnil(L); return 1; }

    auto& col = obj->getComponent<VisualLC>()->curCollision;
    if (idx < 0 || idx >= (int)col.size()) { lua_pushnil(L); return 1; }

    auto it = col.begin();
    std::advance(it, idx);
    lua_pushlightuserdata(L, *it);

    return 1;
}


/* ---- weapon ---- */
int LuaSystem::Lua_getAimAtMouse(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<WeaponComponent>()->aimAtMouse ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getWeaponBaseOffsetX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WeaponComponent>()->baseLocalOffsetX);

    return 1;
}

int LuaSystem::Lua_setWeaponBaseOffsetX(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) return 0;

    obj->getComponent<WeaponComponent>()->baseLocalOffsetX = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}

int LuaSystem::Lua_getLocalOffsetY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Transform>()->localOffset.y);

    return 1;
}

int LuaSystem::Lua_isWeaponOffsetInitialised(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<WeaponComponent>()->baseLocalOffsetInitialised ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setWeaponOffsetInitialised(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) return 0;

    obj->getComponent<WeaponComponent>()->baseLocalOffsetInitialised = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_getWeaponCurrentAngle(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WeaponComponent>()->currentAngle);

    return 1;
}

int LuaSystem::Lua_setWeaponCurrentAngle(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WeaponComponent>()) return 0;

    obj->getComponent<WeaponComponent>()->currentAngle = static_cast<float>(lua_tonumber(L, 2));

    return 0;
}


/* ---- water body ---- */
int LuaSystem::Lua_hasWaterBody(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));

    lua_pushboolean(L, (obj && obj->hasComponent<WaterBody>()) ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_getWaterWidth(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WaterBody>()->width);
    return 1;
}

int LuaSystem::Lua_getWaterHeight(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WaterBody>()->height);
    return 1;
}

int LuaSystem::Lua_getWaterBuoyancyForce(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WaterBody>()->buoyancyForce);
    return 1;
}

int LuaSystem::Lua_getWaterDrag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WaterBody>()->waterDrag);
    return 1;
}

int LuaSystem::Lua_getWaterLastFrameRise(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<WaterBody>()->lastFrameRise);
    return 1;
}

int LuaSystem::Lua_addWaterHeight(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<WaterBody>()) return 0;
    WaterBody* w = obj->getComponent<WaterBody>();
    w->pendingHeightIncrease += w->heightIncrease;
    return 0;
}


/* ---- editor ---- */
int LuaSystem::Lua_isEditingMode(lua_State* L) {
    lua_pushboolean(L, EditorManager::isEditingMode() ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isPaused(lua_State* L) {
    lua_pushboolean(L, EditorManager::isPaused() ? 1 : 0);
    return 1;
}

int LuaSystem::Lua_isEditorOpen(lua_State* L) {
#ifdef _DEBUG
    lua_pushboolean(L, UISystem::isShowUI() ? 1 : 0);
#else
    lua_pushboolean(L, 0);
#endif
    return 1;
}

int LuaSystem::Lua_isSceneHovered(lua_State* L) {
#ifdef _DEBUG
    lua_pushboolean(L, SceneWindow::isSceneHovered() ? 1 : 0);
#else
    lua_pushboolean(L, 1); // in release, always true (no editor panels)
#endif
    return 1;
}


/* ---- ledge climb ---- */
int LuaSystem::Lua_getLedgeClimb(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushboolean(L, 0); return 1; }

    lua_pushboolean(L, obj->getComponent<Physics>()->ledgeClimb ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_setLedgeClimb(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) return 0;

    obj->getComponent<Physics>()->ledgeClimb = lua_toboolean(L, 2) != 0;

    return 0;
}

int LuaSystem::Lua_getLedgeTargetY(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->ledgeTargetY);

    return 1;
}

int LuaSystem::Lua_getLedgeClimbSpeed(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->ledgeClimbSpeed);

    return 1;
}

int LuaSystem::Lua_getJumpBufferTime(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Physics>()) { lua_pushnumber(L, 0.1); return 1; }

    lua_pushnumber(L, obj->getComponent<Physics>()->jumpBufferTime);

    return 1;
}

/* ---- debug ---- */
int LuaSystem::Lua_debugLog(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (!msg) return 0;

    DebugLog::addMessage(msg);

    return 0;
}

int LuaSystem::Lua_debugLogPlay(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (!msg) return 0;

    DebugLog::addMessage(msg, DebugMode::PlaySimul);

    return 0;
}

/* ---- object things ---- */
int LuaSystem::Lua_getAllObjects(lua_State* L) {
    GameObjectManager* mgr = static_cast<GameObjectManager*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!mgr) { lua_newtable(L); return 1; }

    std::vector<GameObject*> gameObjects;
    mgr->getAllGameObjects(gameObjects);

    lua_newtable(L);
    for (int i = 0; i < (int)gameObjects.size(); ++i) {
        lua_pushlightuserdata(L, gameObjects[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int LuaSystem::Lua_getGameObject(lua_State* L) {
    GameObjectManager* mgr = static_cast<GameObjectManager*>(lua_touserdata(L, lua_upvalueindex(1)));
    const char* name = lua_tostring(L, 1);
    if (!mgr || !name) { lua_pushnil(L); return 1; }

    GameObject* obj = mgr->getGameObject(name);

    if (obj) lua_pushlightuserdata(L, obj);
    else     lua_pushnil(L);

    return 1;
}

int LuaSystem::Lua_getObjectName(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj) { lua_pushstring(L, ""); return 1; }

    lua_pushstring(L, obj->getObjectName().c_str());

    return 1;
}

int LuaSystem::Lua_getCollisionTag(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<CollisionInfo>()) { lua_pushstring(L, ""); return 1; }

    lua_pushstring(L, obj->getComponent<CollisionInfo>()->collisionTag.c_str());

    return 1;
}

int LuaSystem::Lua_hasComponent_Physics(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));

    lua_pushboolean(L, (obj && obj->hasComponent<Physics>()) ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_hasComponent_Transform(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));

    lua_pushboolean(L, (obj && obj->hasComponent<Transform>()) ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_hasWeaponComponent(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));

    lua_pushboolean(L, (obj && obj->hasComponent<WeaponComponent>()) ? 1 : 0);

    return 1;
}

int LuaSystem::Lua_getParentName(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj || !obj->hasComponent<Transform>()) { lua_pushstring(L, ""); return 1; }
    lua_pushstring(L, obj->getComponent<Transform>()->parentName.c_str());
    return 1;
}

int LuaSystem::Lua_cloneGameObject(lua_State* L) {
    GameObjectManager* mgr = static_cast<GameObjectManager*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!mgr) { lua_pushnil(L); return 1; }

    const char* srcName = lua_tostring(L, 1);
    const char* dstName = lua_tostring(L, 2);

    if (!srcName || !dstName) { lua_pushnil(L); return 1; }
    GameObject* cloned = mgr->CloneGameObject(srcName, dstName);

    if (cloned) lua_pushlightuserdata(L, cloned);
    else lua_pushnil(L);

    return 1;
}

int LuaSystem::Lua_renameGameObject(lua_State* L) {
    GameObjectManager* mgr = static_cast<GameObjectManager*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!mgr) return 0;

    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    const char* name = lua_tostring(L, 2);

    if (!obj || !name) return 0;
    mgr->renameGameObject(obj, name);

    return 0;
}

int LuaSystem::Lua_setActive(lua_State* L) {
    GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
    if (!obj) return 0;

    obj->active = lua_toboolean(L, 2) != 0;

    return 0;
}



void LuaSystem::registerManagerBindings(GameObjectManager& manager) {
    if (!L) return;

    // need manager to get game object
    lua_pushlightuserdata(L, &manager);
    lua_pushcclosure(L, Lua_getGameObject, 1);
    lua_setglobal(L, "getGameObject");

    lua_pushlightuserdata(L, &manager);
    lua_pushcclosure(L, Lua_getAllObjects, 1);
    lua_setglobal(L, "getAllObjects");

    lua_pushlightuserdata(L, &manager);
    lua_pushcclosure(L, Lua_cloneGameObject, 1);
    lua_setglobal(L, "cloneGameObject");

    lua_pushlightuserdata(L, &manager);
    lua_pushcclosure(L, Lua_renameGameObject, 1);
    lua_setglobal(L, "renameGameObject");

    // things that doesnt need manager
    lua_register(L, "getObjectName", Lua_getObjectName);
    lua_register(L, "getCollisionTag", Lua_getCollisionTag);
    lua_register(L, "hasPhysics", Lua_hasComponent_Physics);
    lua_register(L, "hasTransform", Lua_hasComponent_Transform);
    lua_register(L, "getParentName", Lua_getParentName);
    lua_register(L, "hasWeaponComponent", Lua_hasWeaponComponent);
    lua_register(L, "setActive", Lua_setActive);
}


void LuaSystem::registerGameBinding(const std::string& name, lua_CFunction fn) {
    if (!L) return;
    lua_register(L, name.c_str(), fn);
}


bool LuaSystem::loadScriptForObject(const std::string& filename, const std::string& tableName) {
    if (!L) return false;

    //reload only if file modified
    static std::unordered_map<std::string, std::filesystem::file_time_type> fileTimes;
    auto currentTime = std::filesystem::last_write_time(filename);
    bool reload = false;

    //check if file have changed to know if need reload
    if (fileTimes.find(filename) == fileTimes.end() || fileTimes[filename] != currentTime) {
        fileTimes[filename] = currentTime;
        reload = true;
    }

    if (reload) {
        //load and execute the script file
        if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
            std::cerr << "Lua error loading " << filename << ": "
                << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        // The script should return a table, which is now on top of the stack
        if (!lua_istable(L, -1)) {
            std::cerr << "[Lua] Error: Script " << filename
                << " did not return a table. Make sure it has 'return tableName' at the end."
                << std::endl;
            lua_pop(L, 1);
            return false;
        }

        // Set the returned table as a global with the object name
        lua_setglobal(L, tableName.c_str());

        std::cout << "[Lua] Reloaded script: " << filename << std::endl;
    }

    return true;
}


bool LuaSystem::updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime) {
    if (!L || !obj) return false;

    lua_getglobal(L, tableName.c_str()); //get table

    lua_getfield(L, -1, "Update");       //get table.Update from lua script
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return false;
    }

    lua_pushlightuserdata(L, obj);
    lua_pushnumber(L, deltaTime);

    //call the Update function with 2 arguments and 0 return values
    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        std::cerr << "Lua Update error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);  // add this — pop the error message
    }

    lua_pop(L, 1); //pop table

    return true;
}

//lua updates for all objects with LuaScript component
void LuaSystem::update(GameObjectManager& manager, float deltaTime) {
    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    for (GameObject* obj : gameObjects) {
        // skip player update (its handled by onPlayerInput in luagamescript)
        if (obj->hasComponent<Input>()) continue;

        if (obj->hasComponent<LuaScript>()) {
            LuaScript* scriptComp = obj->getComponent<LuaScript>();

			for (LuaScript::ScriptEntry& scriptEntry : scriptComp->scripts) {
				if (!scriptEntry.scriptName.empty() && !scriptEntry.filename.empty()) {
					// reload script if changed
					if (scriptEntry.changed || scriptComp->scriptsChanged) {
						loadScriptForObject(scriptEntry.filename, scriptEntry.scriptName);
						scriptEntry.changed = false;
					}

                    // update script
					updateObjectScript(scriptEntry.scriptName, obj, deltaTime);
				}
			}

			scriptComp->scriptsChanged = false;
        }
    }
}

void LuaSystem::callSceneLoad(GameObjectManager& manager) {
    if (!L) return;
    std::string path = std::string(RUNTIME_SCRIPT_DIR_R) + "/scene.lua";
    luaL_dofile(L, path.c_str());
    lua_getglobal(L, "onSceneLoad");
    if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return; }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << "Lua onSceneLoad error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
}