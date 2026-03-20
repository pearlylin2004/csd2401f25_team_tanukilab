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
    lua_register(L, "getFireRate", Lua_getFireRate);

    // physics setters
    lua_register(L, "setVelX", Lua_setVelX);
    lua_register(L, "setVelY", Lua_setVelY);
    lua_register(L, "setOnGround", Lua_setOnGround);
    lua_register(L, "setCanMove", Lua_setCanMove);
    lua_register(L, "setGravity", Lua_setGravity);
    lua_register(L, "setShootTimer", Lua_setShootTimer);
    lua_register(L, "setPhysicsFlag", Lua_setPhysicsFlag);

    // transform
    lua_register(L, "setFlipX", Lua_setFlipX);
    lua_register(L, "getFlipX", Lua_getFlipX);

    // input bindings
    lua_register(L, "isKeyTriggered", Lua_isKeyTriggered);
    lua_register(L, "isMouseLeftHeld", Lua_isMouseLeftHeld);
    lua_register(L, "isMouseRightHeld", Lua_isMouseRightHeld);
    lua_register(L, "isMouseLeftTriggered", Lua_isMouseLeftTriggered);
    lua_register(L, "getMouseWorldX", Lua_getMouseWorldX);
    lua_register(L, "getMouseWorldY", Lua_getMouseWorldY);

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


/* ---- transform ---- */
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


/* ---- object things ---- */
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

void LuaSystem::registerManagerBindings(GameObjectManager& manager) {
    if (!L) return;

    // need manager to get game object
    lua_pushlightuserdata(L, &manager);
    lua_pushcclosure(L, Lua_getGameObject, 1);
    lua_setglobal(L, "getGameObject");

    // things that doesnt need manager
    lua_register(L, "getObjectName", Lua_getObjectName);
    lua_register(L, "getCollisionTag", Lua_getCollisionTag);
    lua_register(L, "hasPhysics", Lua_hasComponent_Physics);
    lua_register(L, "hasTransform", Lua_hasComponent_Transform);
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
