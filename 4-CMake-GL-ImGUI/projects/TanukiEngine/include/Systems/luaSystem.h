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


	/* -------- Input Control -------- */
	static int Lua_getPosition(lua_State* L);
	static int Lua_setPosition(lua_State* L);
	static int Lua_IsKeyHeld(lua_State* L);
	static int Lua_SendInputEvent(lua_State* L);


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
	static int Lua_getFireRate(lua_State* L);
	

	/* -------- Physics Setters -------- */
	static int Lua_setVelX(lua_State* L);
	static int Lua_setVelY(lua_State* L);
	static int Lua_setOnGround(lua_State* L);
	static int Lua_setCanMove(lua_State* L);
	static int Lua_setGravity(lua_State* L);
	static int Lua_setShootTimer(lua_State* L);
	static int Lua_setPhysicsFlag(lua_State* L);


	/* -------- Transform Getters -------- */
	static int Lua_setFlipX(lua_State* L);
	static int Lua_getFlipX(lua_State* L);


	/* -------- Input Bindings ---- */
	static int Lua_isKeyTriggered(lua_State* L);
	static int Lua_isMouseLeftHeld(lua_State* L);
	static int Lua_isMouseRightHeld(lua_State* L);
	static int Lua_isMouseLeftTriggered(lua_State* L);
	static int Lua_getMouseWorldX(lua_State* L);
	static int Lua_getMouseWorldY(lua_State* L);


	/* -------- Object Things ---- */
	static int Lua_getGameObject(lua_State* L);
	static int Lua_getObjectName(lua_State* L);
	static int Lua_getCollisionTag(lua_State* L);
	static int Lua_hasComponent_Physics(lua_State* L);
	static int Lua_hasComponent_Transform(lua_State* L);


private:
	lua_State* L = nullptr; //pointer to Lua state
	MessageBus* messageBus = nullptr; //pointer to message bus
	//std::unique_ptr<GameObjectManager> manager; //pointer to game object manager
	GUISystem* guiSystem = nullptr; //pointer to GUI system

	void registerManagerBindings(GameObjectManager& manager);
};
