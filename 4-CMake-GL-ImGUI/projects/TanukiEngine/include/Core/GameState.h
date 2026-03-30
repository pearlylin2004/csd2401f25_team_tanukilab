/* Start Header ************************************************************************/
/*!
\file       GameState.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       January, 30th, 2026
\brief      

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

enum class GameState {
    NONE,
    MENU,
    PLAYING,
    CONTROL,
    AUDIO,
    PAUSED,
    EXIT,
    EXITCONFIRMATION,
    LEVELS,//maybe for future use
    WIN,
    LEADERBOARD,
    REPLAY
};