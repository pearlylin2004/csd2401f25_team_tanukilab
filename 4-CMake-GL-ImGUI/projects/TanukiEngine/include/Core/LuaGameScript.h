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
        m_lua.callSceneLoad(manager);
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
            if (!entry.scriptName.empty() && !entry.filename.empty())
                m_lua.updateObjectScript(entry.scriptName, &obj, dt);
        }
    }

    void updateAll(GameObjectManager& manager, float dt) override {
        m_lua.update(manager, dt);
    }

private:
    LuaSystem& m_lua;
};