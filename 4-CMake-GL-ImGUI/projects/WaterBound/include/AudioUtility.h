#pragma once
#include <../lib/FMOD/fmod.hpp>
#include <../lib/FMOD/fmod_errors.h>
#include <iostream>
#include "Editor/gameDebugLog.h"

inline void FMODErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
        DebugLog::addMessage("Audio error: " + std::string(FMOD_ErrorString(result)));
        //exit(-1); // instead of terminate, can change to logging the errors
    }
}