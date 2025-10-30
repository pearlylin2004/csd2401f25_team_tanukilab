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
    luaL_openlibs(L);
}

void LuaSystem::cleanup() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
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