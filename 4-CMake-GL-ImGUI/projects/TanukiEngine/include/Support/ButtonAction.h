/* Start Header ************************************************************************/
/*!
\file       ButtonAction.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       March, 27th, 2025
\brief      

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

enum class ButtonAction {
    NONE,
    LOAD_SCENE,
    PAUSE, // like load scene but preserve scene before pause
    RESTART,
    EXIT_GAME,
    //RESUME_GAME,
    BACK,
    MUSIC_VOL_UP,
    MUSIC_VOL_DOWN,
    MUTE,
    UNMUTE
};

//enum for button actions
namespace std {
    template <>
    struct hash<ButtonAction> {
        size_t operator()(const ButtonAction& b) const noexcept {
            return std::hash<int>()(static_cast<int>(b));
        }
    };
}