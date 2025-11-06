/* Start Header ************************************************************************/
/*!
\file       messageBus.h
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
#include <unordered_map>
#include <vector>
#include <algorithm>

//for advance messaging
struct KeyEvent {
    std::string key;
    bool pressed; // true = pressed, false = released
    //for lua
    float x = 0.0f;
    float y = 0.0f;
};

class MessageBus {
public:
    void subscribe(const std::string& messageType, subscriber* subscriber) {
        subscribers[messageType].push_back(subscriber);
    }

    void unsubscribe(const std::string& messageType, subscriber* subscriber) {
        auto& subs = subscribers[messageType];
        subs.erase(std::remove(subs.begin(), subs.end(), subscriber), subs.end());
    }

    void publish(const Message& message) {
        auto it = subscribers.find(message.type);
        if (it != subscribers.end()) {
            for (auto* sub : it->second)
                sub->onNotify(message);
        }
    }
private:
    std::unordered_map<std::string, std::vector<subscriber*>> subscribers;
};
