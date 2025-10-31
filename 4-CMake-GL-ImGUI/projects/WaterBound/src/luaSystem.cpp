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

void LuaSystem::init() {
    L = luaL_newstate();
    lua_register(L, "Input_isKeyHeld", Lua_IsKeyHeld);
    lua_register(L, "getPosition", Lua_getPosition);
    lua_register(L, "setPosition", Lua_setPosition);
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

    //convert string to GLFW key code
    int key = -1;
    if (strcmp(keyStr, "W") == 0) key = GLFW_KEY_W;
    else if (strcmp(keyStr, "A") == 0) key = GLFW_KEY_A;
    else if (strcmp(keyStr, "S") == 0) key = GLFW_KEY_S;
    else if (strcmp(keyStr, "D") == 0) key = GLFW_KEY_D;
    else if (strcmp(keyStr, "SPACE") == 0) key = GLFW_KEY_SPACE;
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
    //float z = static_cast<float>(lua_tonumber(L, 4));

    transform->x = x;
    transform->y = y;
    //transform->z = z;

    return 0;
}

bool LuaSystem::runScriptForObject(const std::string& filename, GameObject* obj, float deltaTime) {
    if (!L || !obj) return false;
    /*
    //load the script only once
    static std::unordered_map<std::string, bool> loadedScripts;
    if (loadedScripts.find(filename) == loadedScripts.end()) {
        if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
            std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }
        loadedScripts[filename] = true;
    }*/

    //load the Lua script from file every time
    if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }

    //expose deltaTime
    lua_pushnumber(L, deltaTime);
    lua_setglobal(L, "deltaTime");

    //expose object
    lua_pushlightuserdata(L, obj);
    lua_setglobal(L, "object");

    //expose input
    //lua_pushcclosure(L, Lua_IsKeyHeld, 0);
    //lua_setglobal(L, "Input_isKeyHeld");

    //call Lua function Update(object, deltaTime)
    lua_getglobal(L, "Update");
    if (lua_isfunction(L, -1)) {
        lua_pushlightuserdata(L, obj);
        lua_pushnumber(L, deltaTime);
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            std::cerr << "Lua Update error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    else {
        lua_pop(L, 1); //pop non-function
    }
    return true;
}