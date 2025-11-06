/* Start Header ************************************************************************/
/*!
\file       controllerSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       November, 4th, 2025
\brief

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "subscriber.h"
#include "luaSystem.h"
#include <iostream>

//for input
class PlayerControllerSystem : public subscriber {
public:
	void onNotify(const Message& msg) override {
		if (msg.type == "KeyPressed") {
			auto keyEvent = std::any_cast<KeyEvent>(msg.payload);
			if (keyEvent.key == "SPACE") {
				std::cout << "Bullet shooting!\n";
			}
			else if (keyEvent.key == "W") {
				std::cout << "Player moving upwards!\n";
			}
			else if (keyEvent.key == "S") {
				std::cout << "Player moving downwards!\n";
			}
			else if (keyEvent.key == "A") {
				std::cout << "Player moving left side!\n";
			}
			else if (keyEvent.key == "D") {
				std::cout << "Player moving right side!\n";
			}
			else if (keyEvent.key == "0") {
				std::cout << "Toggling UI!\n";
			}
            else if (keyEvent.key == "B") {
                std::cout << "Player is jumping!\n";
            }
		}
	}
};

//for lua
class LuaSubscriber : public subscriber {
public:
    LuaSubscriber(lua_State* L) : L(L) {}

    void onNotify(const Message& msg) override {
        if (!L) return;

        //push global Lua function if it exists
        lua_getglobal(L, "onInputEvent");

        if (lua_isfunction(L, -1)) {
            try {
                auto keyEvent = std::any_cast<KeyEvent>(msg.payload);

                lua_pushstring(L, msg.type.c_str());
                lua_pushstring(L, keyEvent.key.c_str());
                lua_pushnumber(L, keyEvent.x); 
                lua_pushnumber(L, keyEvent.y);

                //call Lua function (4 args, 0 return)
                if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
                    std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
                    lua_pop(L, 1);
                }
            }
            catch (const std::bad_any_cast&) {
                std::cerr << "Invalid KeyEvent payload in LuaSubscriber\n";
            }
        }
        else {
            lua_pop(L, 1); //remove non-function
        }
    }

private:
    lua_State* L;
};
