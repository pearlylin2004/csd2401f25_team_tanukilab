/* Start Header ************************************************************************/
/*!
\file       ButtonSystem.h
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
    START_GAME,
    OPEN_CONTROL,
    OPEN_AUDIO,
    EXIT_GAME,
    RESUME_GAME,
    YES_EXIT,
    NO_EXIT,
    BACK,
    MENU,
    MUSIC_VOL_UP,
    MUSIC_VOL_DOWN,
    SFX_VOL_UP,
    SFX_VOL_DOWN,
    MUTE,
    UNMUTE
};