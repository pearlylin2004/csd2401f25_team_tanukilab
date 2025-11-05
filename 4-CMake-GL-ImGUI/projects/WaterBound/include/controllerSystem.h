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
#include <iostream>

//for advance messaging
struct KeyEvent {
	std::string key;
	bool pressed; // true = pressed, false = released
};

class PlayerControllerSystem : public subscriber {
public:
	void onNotify(const Message& msg) override {
		if (msg.type == "KeyPressed") {
			auto keyEvent = std::any_cast<KeyEvent>(msg.payload);
			if (keyEvent.key == "SPACE") {
				std::cout << "Bullet shooting!\n";
			}
		}
	}
};
