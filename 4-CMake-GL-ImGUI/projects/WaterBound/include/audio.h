/* Start Header ************************************************************************/
/*!
\file       audio.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 20th, 2025
\brief      This header file declares the AudioHandler class, a singleton wrapper around
			the FMOD library for managing sound in the project. It provides function
			prototypes for initializing and shutting down the audio system, loading sounds,
			playing them and controlling playback features such as pause, stop, mute,
			volume and pitch. The soundID enum is defined to categorize different sounds.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <../lib/FMOD/fmod.hpp>
//#include <../lib/FMOD/fmod_errors.h>
#include <vector>
#include <string>
#include <iostream>

#include "AudioUtility.h"
#include "ResourceManager.h"
#include "performance.h"

enum class AudioState {
	Stopped,
	Playing,
	Paused
};

enum class soundID : int {
	water = 0,
	shoot,
	fire,
	bg,
	// keep this as the last, for getting the number of sound ids
	count
};

struct FadeInfo {
	bool isFading = false;
	bool isFadingIn = false;
	float startVolume = 0.0f;
	float targetVolume = 0.0f;
	float fadeDuration = 0.0f;
	float fadeTimer = 0.0f;

	float defaultFadeInDuration = 1.0f;
	float defaultFadeOutDuration = 1.0f;
};


class AudioHandler {
public:
	// get the singleton instance
	static AudioHandler& getInstance(); // make sure only 1 instance is created

	// check for FMOD errors (future: can change to logging instead of exit(-1))
	void init();
	// initialize the audio system and load sounds
	void update(float deltaTime);
	// release all sounds and shut down the audio system (will shutdown automatically when dtor called)
	void shutdown(); // no need to manually call, dtor will call this automatically

	// load the sound from file path, if loop is true, sound will loop until stopped (default false)
	// now done in ResourceManager
	// bool loadSound(soundID id, const std::string& filePath, bool loop = false);
	
	// play the sound, return the channel pointer if successful, nullptr if failed
	FMOD::Channel* playSound(soundID id, float volume = 1.0f);

	// call this to PAUSE / RESUME sound
	void toggleSoundPause(soundID id);
	// use this to PAUSE / RESUME ALL sounds
	void toggleAllSoundPause(); 
	// use this to check whether sound is paused
	bool isSoundPaused(soundID id); 

	// to stop a particular sound
	void stopSound(soundID id); 
	// to check whether channel still alive (ie. sound finished playing)
	bool isSoundPlaying(soundID id); 

	// mute specific sound
	void muteSound(soundID id); 
	// mute all sounds
	void muteAllSound(); 
	// unmute specific sound
	void unmuteSound(soundID id); 
	// unmute all sounds
	void unmuteAllSound(); 
	// increase by 0.1f
	void increaseSoundVolume(soundID id); 
	// decrease by 0.1f
	void decreaseSoundVolume(soundID id); 
	// set a specific volume
	void setSoundVolume(soundID id, float volume); // 0.0f (muted) to 1.0f (full)

	// set a specific pitch
	void setSoundpitch(soundID id, float pitch); // 0.5f (half speed) to 2.0f (double speed)

	//pause and resume
	void pauseAll();
	void resumeAll();

	void fadeIn(soundID id, float targetVolume = 1.0f, float duration = 1.0f);
	void fadeOut(soundID id, float duration = 1.0f);
	bool isFading(soundID id) const;
	void stopFade(soundID id);

	// perform error check for FMOD functions
	// moved to AudioUtility.h
	//static void FMODErrorCheck(FMOD_RESULT result);

private: 
	AudioHandler() : system(nullptr) {}
	~AudioHandler() { shutdown(); } // auto shutdown once instance is destroyed

	// helper function to convert soundID to file path
	// currently hardcoded the path, to change once deserialization is implemented
	std::string soundIDToFilePath(soundID id);

	void updateFades(float deltaTime);

	FMOD::System* system;
	//std::vector<FMOD::Sound*> sounds;
	std::vector<FMOD::Channel*> channels;
	std::vector<FadeInfo> fadeInfo;
};