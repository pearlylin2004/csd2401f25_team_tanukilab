/* Start Header ************************************************************************/
/*!
\file       luaSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Ocotber, 23rd, 2025
\brief      declaration of the LuaSystem class to handle Lua scripting integration

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <iostream>
#include "GameObject.h"
#include "input.h"
#include "Component.h"
#include "messageBus.h"
#include "GameObjectManager.h"
#include "GUISystem.h"
#include "Physics.h"
#include "Editor/editorManager.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class LuaSystem {
public:
    LuaSystem() = default;
    ~LuaSystem() = default;

	void init(GameObjectManager& manager); //initialize Lua state
    void cleanup(); //clean up Lua state
    bool runString(const std::string& code); //execute Lua code from a string
	bool runScript(const std::string& filename); //execute Lua code from a file
	float getGlobalNumber(const std::string& name); //to get the global number variable from Lua
    bool updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime); //update the script 
    bool loadScriptForObject(const std::string& filename, const std::string& tableName); //load script
	void setMessageBus(MessageBus* bus) { messageBus = bus; } //set message bus
	void update(GameObjectManager& manager, float deltaTime); //update all Lua scripts


	/* -------- On Scene Load ---- */
	void callSceneLoad(GameObjectManager& manager);

	/* -------- Register Game Script -------- */
	void registerGameBinding(const std::string& name, lua_CFunction fn);
	lua_State* getLuaState() { return L; }


	/* -------- Input Control -------- */
	static int Lua_getPosition(lua_State* L);
	static int Lua_setPosition(lua_State* L);
	static int Lua_IsKeyHeld(lua_State* L);
	static int Lua_SendInputEvent(lua_State* L);


	/* -------- Render -------- */
	static int Lua_setRenderAlpha(lua_State* L);
	static int Lua_setTexture(lua_State* L);
	static int Lua_setColliderSize(lua_State* L);


	/* -------- Physics Getters -------- */
	static int Lua_getVelX(lua_State* L);
	static int Lua_getVelY(lua_State* L);
	static int Lua_isOnGround(lua_State* L);
	static int Lua_getCanMove(lua_State* L);
	static int Lua_getMoveSpeed(lua_State* L);
	static int Lua_getJumpForce(lua_State* L);
	static int Lua_isInWater(lua_State* L);
	static int Lua_getGravity(lua_State* L);
	static int Lua_canShoot(lua_State* L);
	static int Lua_getShootTimer(lua_State* L);
	static int Lua_isAlive(lua_State* L);
	static int Lua_getFireRate(lua_State* L);
	static int Lua_getLifeTimer(lua_State* L);
	static int Lua_getMaxLifetime(lua_State* L);
	static int Lua_getGravityDelayTimer(lua_State* L);
	static int Lua_getGravityDelay(lua_State* L);
	static int Lua_getInitialScale(lua_State* L);
	static int Lua_getMinScale(lua_State* L);
	static int Lua_getDynVelX(lua_State* L);
	static int Lua_getDynVelY(lua_State* L);
	static int Lua_getDynPosX(lua_State* L);
	static int Lua_getDynPosY(lua_State* L);
	static int Lua_getDynMass(lua_State* L);
	static int Lua_getCoyoteTime(lua_State* L);
	static int Lua_getMaxFallSpeed(lua_State* L);
	static int Lua_getSinlePersonalBool(lua_State* L);
	static int Lua_getSinleFlag(lua_State* L);
	static int Lua_getHitground(lua_State* L);
	static int Lua_getSinleGravity(lua_State* L);
	static int Lua_getWaterBounce(lua_State* L);
	static int Lua_getAccelX(lua_State* L);
	static int Lua_getAccelY(lua_State* L);


	/* -------- Physics Setters -------- */
	static int Lua_setVelX(lua_State* L);
	static int Lua_setVelY(lua_State* L);
	static int Lua_setOnGround(lua_State* L);
	static int Lua_setCanMove(lua_State* L);
	static int Lua_setGravity(lua_State* L);
	static int Lua_setShootTimer(lua_State* L);
	static int Lua_setPhysicsFlag(lua_State* L);
	static int Lua_applyDamping(lua_State* L);
	static int Lua_updatePosition(lua_State* L);
	static int Lua_setAlive(lua_State* L);
	static int Lua_setLifeTimer(lua_State* L);
	static int Lua_setGravityDelayTimer(lua_State* L);
	static int Lua_setDynVelX(lua_State* L);
	static int Lua_setDynVelY(lua_State* L);
	static int Lua_setDynPos(lua_State* L);
	static int Lua_updateForces(lua_State* L);
	static int Lua_advanceDynamics(lua_State* L);
	static int Lua_setInWater(lua_State* L);
	static int Lua_clearDynForces(lua_State* L);
	static int Lua_setOriginalPos(lua_State* L);
	static int Lua_isOriginalStateSet(lua_State* L);
	static int Lua_setAutoUpdate(lua_State* L);
	static int Lua_setAccelX(lua_State* L);
	static int Lua_setAccelY(lua_State* L);
	static int Lua_setHitground(lua_State* L);


	/* -------- Collision -------- */
	static int Lua_getCollisionFlag(lua_State* L);
	static int Lua_setCollisionFlag(lua_State* L);
	static int Lua_setCollisionTag(lua_State* L);
	static int Lua_getFriction(lua_State* L);


	/* -------- Transform Getters -------- */
	static int Lua_getRotation(lua_State* L);
	static int Lua_setRotation(lua_State* L);
	static int Lua_setScale(lua_State* L);
	static int Lua_getScaleX(lua_State* L);
	static int Lua_getScaleY(lua_State* L);
	static int Lua_getLocalOffsetX(lua_State* L);
	static int Lua_setLocalOffsetX(lua_State* L);
	static int Lua_setFlipX(lua_State* L);
	static int Lua_getFlipX(lua_State* L);


	/* -------- Input Bindings -------- */
	static int Lua_isKeyTriggered(lua_State* L);
	static int Lua_isKeyReleased(lua_State* L);
	static int Lua_isMouseLeftHeld(lua_State* L);
	static int Lua_isMouseRightHeld(lua_State* L);
	static int Lua_isMouseLeftTriggered(lua_State* L);
	static int Lua_getMouseWorldX(lua_State* L);
	static int Lua_getMouseWorldY(lua_State* L);
	static int Lua_getEditorMouseWorldX(lua_State* L);
	static int Lua_getEditorMouseWorldY(lua_State* L);


	/* -------- FSM -------- */
	static int Lua_getPlayerState(lua_State* L);
	static int Lua_setPlayerState(lua_State* L);
	static int Lua_isFixState(lua_State* L);
	static int Lua_setFixState(lua_State* L);
	static int Lua_getFacingRight(lua_State* L);
	static int Lua_setFacingRight(lua_State* L);
	static int Lua_getStateTime(lua_State* L);
	static int Lua_setStateTime(lua_State* L);


	/* -------- Audio -------- */
	static int Lua_playSound(lua_State* L);
	static int Lua_stopSound(lua_State* L);
	static int Lua_setSoundVolume(lua_State* L);
	static int Lua_isSoundPlaying(lua_State* L);


	/* -------- Visual Script -------- */
	static int Lua_getVSGlobalFloat(lua_State* L);
	static int Lua_setVSGlobalFloat(lua_State* L);
	static int Lua_hasVSGlobalFloat(lua_State* L);
	static int Lua_getNumCurCollisions(lua_State* L);
	static int Lua_getCurCollision(lua_State* L);


	/* -------- Weapon -------- */
	static int Lua_getAimAtMouse(lua_State* L);
	static int Lua_getWeaponBaseOffsetX(lua_State* L);
	static int Lua_setWeaponBaseOffsetX(lua_State* L);
	static int Lua_getLocalOffsetY(lua_State* L);
	static int Lua_isWeaponOffsetInitialised(lua_State* L);
	static int Lua_setWeaponOffsetInitialised(lua_State* L);
	static int Lua_getWeaponCurrentAngle(lua_State* L);
	static int Lua_setWeaponCurrentAngle(lua_State* L);


	/* -------- WaterBody -------- */
	static int Lua_hasWaterBody(lua_State* L);
	static int Lua_getWaterWidth(lua_State* L);
	static int Lua_getWaterHeight(lua_State* L);
	static int Lua_getWaterBuoyancyForce(lua_State* L);
	static int Lua_getWaterDrag(lua_State* L);
	static int Lua_getWaterLastFrameRise(lua_State* L);
	static int Lua_addWaterHeight(lua_State* L);


	/* -------- Editor State -------- */
	static int Lua_isEditingMode(lua_State* L);
	static int Lua_isPaused(lua_State* L);
	static int Lua_isEditorOpen(lua_State* L);
	static int Lua_isSceneHovered(lua_State* L);


	/* -------- Ledge Climb -------- */
	static int Lua_getLedgeClimb(lua_State* L);
	static int Lua_setLedgeClimb(lua_State* L);
	static int Lua_getLedgeTargetY(lua_State* L);
	static int Lua_getLedgeClimbSpeed(lua_State* L);
	static int Lua_getJumpBufferTime(lua_State* L);


	/* -------- Debug -------- */
	static int Lua_debugLog(lua_State* L); // print message in play simulation debug log
	static int Lua_debugLogPlay(lua_State* L);


	/* -------- Object Things ---- */
	static int Lua_getAllObjects(lua_State* L);
	static int Lua_getGameObject(lua_State* L);
	static int Lua_getObjectName(lua_State* L);
	static int Lua_getCollisionTag(lua_State* L);
	static int Lua_hasComponent_Physics(lua_State* L);
	static int Lua_hasComponent_Transform(lua_State* L);
	static int Lua_hasWeaponComponent(lua_State* L);
	static int Lua_getParentName(lua_State* L);
	static int Lua_cloneGameObject(lua_State* L);
	static int Lua_renameGameObject(lua_State* L);
	static int Lua_setActive(lua_State* L);


private:
	lua_State* L = nullptr; //pointer to Lua state
	MessageBus* messageBus = nullptr; //pointer to message bus
	//std::unique_ptr<GameObjectManager> manager; //pointer to game object manager
	GUISystem* guiSystem = nullptr; //pointer to GUI system

	void registerManagerBindings(GameObjectManager& manager);
};
