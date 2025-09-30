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
#include <../lib/FMOD/fmod_errors.h>
#include <vector>
#include <string>
#include <iostream>


enum class soundID : int {
	effect = 0, // for water sound (placeholder only)
	bg,
	// keep this as the last, for getting the number of sound ids
	count
};

class AudioHandler {
public:
	static AudioHandler& getInstance(); // make sure only 1 instance is created

	void init();
	void update();
	void shutdown();

	bool loadSound(soundID id, const std::string& filePath, bool loop = false);
	FMOD::Channel* playSound(soundID id, float volume = 1.0f);

	void toggleSoundPause(soundID id); // use this to PAUSE / RESUME sound
	void toggleAllSoundPause(); // use this to PAUSE / RESUME ALL sounds
	bool isSoundPaused(soundID id); // use this to check whether sound is paused

	void stopSound(soundID id); 
	bool isSoundPlaying(soundID id); // to check whether channel still alive (ie. sound finished playing)

	void muteSound(soundID id); // mute specific sound
	void muteAllSound(); // mute all sounds
	void unmuteSound(soundID id); // unmute specific sound
	void unmuteAllSound(); // unmute all sounds
	void increaseSoundVolume(soundID id); // increase by 0.1f
	void decreaseSoundVolume(soundID id); // decrease by 0.1f
	void setSoundVolume(soundID id, float volume); // 0.0f (muted) to 1.0f (full)

	void setSoundpitch(soundID id, float pitch); // 0.5f (half speed) to 2.0f (double speed)

	static void FMODErrorCheck(FMOD_RESULT result);

private: 
	AudioHandler() : system(nullptr) {}
	~AudioHandler() { shutdown(); } // auto shutdown once instance is destroyed

	// helper function to convert soundID to file path
	// currently hardcoded the path, to change once deserialization is implemented
	std::string soundIDToFilePath(soundID id);

	FMOD::System* system;
	std::vector<FMOD::Sound*> sounds;
	std::vector<FMOD::Channel*> channels;
};