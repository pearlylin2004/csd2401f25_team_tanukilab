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

#pragma once
#include <string>
#include <iostream>
#include "GameObject.h"
#include "input.h"
#include "Component.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class LuaSystem {
public:
    LuaSystem() = default;
    ~LuaSystem() = default;

    void init();
    void cleanup();
    bool runString(const std::string& code);
    bool runScript(const std::string& filename);
    float getGlobalNumber(const std::string& name);
    bool updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime);
    bool loadScriptForObject(const std::string& filename, const std::string& tableName);
    static int Lua_getPosition(lua_State* L);
    static int Lua_setPosition(lua_State* L);
    static int Lua_IsKeyHeld(lua_State* L);

private:
    lua_State* L = nullptr;
};
