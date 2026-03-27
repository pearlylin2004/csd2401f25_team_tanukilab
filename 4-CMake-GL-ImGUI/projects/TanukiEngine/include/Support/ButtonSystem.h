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

    // --- Navigation / scene ---
    LOAD_SCENE,     // load the scene stored in Button::scenePath
    GO_BACK,        // return to previous scene / resume gameplay
    EXIT_GAME,      // go to exit-confirmation scene
    YES_EXIT,       // confirm quit
    NO_EXIT,        // cancel quit, return to menu

    // --- Audio ---
    MUSIC_VOL_UP,
    MUSIC_VOL_DOWN,
    MUTE,
    UNMUTE,

    // --- Gameplay ---
    RESTART_LEVEL,  // restart current level (deferred to next frame)
};