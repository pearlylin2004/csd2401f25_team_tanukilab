/* Start Header ************************************************************************/
/*!
\file       ui.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 90%
			Pearly Lin Lee Ying, p.lin, 2401591
			- 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 24th, 2025
\brief      Declaration of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <vector>
#include <string>

enum class DebugMode {
    Editor, // means editing
    PlaySimul // means playing simulation, e.g. player pos etc
};

struct DebugMessage {
    std::string text;
    DebugMode mode;
};

class DebugLog {
public:
    // add a message with current mode
    static void addMessage(const std::string& msg, DebugMode mode = DebugMode::Editor) {
        messages.push_back({ msg, mode });

        if (mode == DebugMode::Editor) {
            // count editor messages
            size_t editorCount = 0;
            for (auto& m : messages)
                if (m.mode == DebugMode::Editor) editorCount++;

            // remove oldest editor messages if over 500
            if (editorCount > 500) {
                auto it = std::find_if(messages.begin(), messages.end(),
                    [](const DebugMessage& m) { return m.mode == DebugMode::Editor; });
                if (it != messages.end()) messages.erase(it);
            }
        }
    }

    // clear all play simulation msg (when simulation stop)
    static void clearPlaySimulMsg() {
        messages.erase(std::remove_if(messages.begin(), messages.end(),
            [](const DebugMessage& msg) { return msg.mode == DebugMode::PlaySimul;}), messages.end());
    }

    // get all messages
    static const std::vector<DebugMessage>& getMessages() {
        return messages;
    }

private:
    static inline std::vector<DebugMessage> messages;
};
