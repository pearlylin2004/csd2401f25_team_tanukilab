/* Start Header ************************************************************************/
/*!
\file        AudioUtility.h
\author      Tan Chee Qing, cheeqing.tan, 2401486
\par         cheeqing.tan@digipen.edu
\date        November, 20th, 2025
\brief       Provides centralized FMOD error checking and logging functionality for the audio system.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <../lib/FMOD/fmod.hpp>
#include <../lib/FMOD/fmod_errors.h>
#include <iostream>
#include "Editor/gameDebugLog.h"

//checks if the file used is valid for FMOD
inline void FMODErrorCheck(FMOD_RESULT result) {
	// FMOD_ERR_INVALID_HANDLE is a common "error" that just means the sound finished and the channel handle is no longer valid
    if (result != FMOD_OK && result != FMOD_ERR_INVALID_HANDLE) {
        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
        DebugLog::addMessage("Audio error: " + std::string(FMOD_ErrorString(result)), DebugMode::PlaySimul);
        //exit(-1); // instead of terminate, can change to logging the errors
    }
}