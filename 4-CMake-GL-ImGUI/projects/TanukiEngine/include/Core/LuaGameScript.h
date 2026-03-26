/* Start Header ************************************************************************/
/*!
\file       LuaGameScript.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       March, 20th, 2026
\brief      

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "GameScript.h"
#include "luaSystem.h"

class LuaGameScript : public GameScript {
public:
    LuaGameScript(LuaSystem& lua) : m_lua(lua) {}

    void onSceneLoad(GameObjectManager& manager) override {
        m_lua.callSceneLoad(manager, manager.getOnLoadScript());
        m_lua.initializeScripts(manager);
    }

    void update(GameObject& obj, float dt) override {
        (void)obj; (void)dt;if (obj.hasComponent<Input>()) return; // handled by onPlayerInput
        if (!obj.hasComponent<LuaScript>()) return;

        LuaScript* script = obj.getComponent<LuaScript>();
        for (auto& entry : script->scripts)
            if (!entry.scriptName.empty() && !entry.filename.empty())
                m_lua.updateObjectScript(entry.scriptName, &obj, dt);
    }

    void updateEnvironment(GameObject& obj, float dt) override {
        if (obj.hasComponent<Input>()) return;
        if (!obj.hasComponent<LuaScript>()) return;

        LuaScript* script = obj.getComponent<LuaScript>();
        for (auto& entry : script->scripts)
            if (!entry.scriptName.empty() && !entry.filename.empty())
                m_lua.updateObjectScript(entry.scriptName, &obj, dt);
    }

    void onPlayerInput(GameObject& obj, GameObjectManager& manager, MessageBus& messageBus, float dt) override {
        (void)manager; (void)messageBus;
        if (!obj.hasComponent<LuaScript>()) return;

        LuaScript* script = obj.getComponent<LuaScript>();
        for (auto& entry : script->scripts) {
            if (!entry.scriptName.empty() && !entry.filename.empty()) {
                if (entry.changed || script->scriptsChanged)
                    m_lua.loadScriptForObject(entry.filename, entry.scriptName);
                m_lua.updateObjectScript(entry.scriptName, &obj, dt);
            }
        }
    }

    void updateAll(GameObjectManager& manager, float dt) override {
        m_lua.update(manager, dt);
    }

    bool onCollision(GameObject* a, GameObject* b, CollisionInfo& info, GameObjectManager& manager) override {
        (void)info; (void)manager;
        lua_State* L = m_lua.getLuaState();
        lua_getglobal(L, "onCollision");
        if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return false; }
        lua_pushlightuserdata(L, a);
        lua_pushlightuserdata(L, b);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            lua_pop(L, 1); return false;
        }
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }

    bool onCollisionResponse(GameObject* obj, CollisionInfo* col, CollisionInfo& info, bool isStatic, CollisionInfo* otherCol) override {
        if (!col || !otherCol) return false;
        lua_State* L = m_lua.getLuaState();
        lua_getglobal(L, "onCollisionResponse");
        if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return false; }
        lua_pushlightuserdata(L, obj);
        lua_pushstring(L, col->collisionTag.c_str());
        lua_pushstring(L, otherCol->collisionTag.c_str());
        lua_pushnumber(L, info.normal.x);
        lua_pushnumber(L, info.normal.y);
        lua_pushboolean(L, isStatic ? 1 : 0);
        lua_pushnumber(L, info.penetration);
        if (lua_pcall(L, 7, 1, 0) != LUA_OK) {
            lua_pop(L, 1); return false;
        }
        bool result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }

    Vector2D getAudioListenerPos(GameObjectManager& manager) override {
        lua_State* L = m_lua.getLuaState();
        lua_getglobal(L, "getAudioListenerPos");
        if (!lua_isfunction(L, -1)) { lua_pop(L, 1); return { 0.f, 0.f }; }
        if (lua_pcall(L, 0, 2, 0) != LUA_OK) {
            lua_pop(L, 1); return { 0.f, 0.f };
        }
        float x = static_cast<float>(lua_tonumber(L, -2));
        float y = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 2);
        return { x, y };
    }

private:
    LuaSystem& m_lua;
};