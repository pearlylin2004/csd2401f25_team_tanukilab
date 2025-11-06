/* Start Header ************************************************************************/
/*!
\file       audio.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 20th, 2025
\brief      This source file implements the AudioHandler singleton class, which wraps
			FMOD functionality for managing audio in the project. It provides methods
			to initialize, update and shut down the audio system, as well as to load,
			play, pause, stop, mute and adjust volume or pitch of sounds identified by
			the soundID enum.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "audio.h"
#include <../lib/FMOD/fmod_errors.h>

// get the singleton instance
AudioHandler& AudioHandler::getInstance() {
	static AudioHandler instance;
	return instance;
}

// check for FMOD errors (future: can change to logging instead of exit(-1))
//void AudioHandler::FMODErrorCheck(FMOD_RESULT result) {
//    if (result != FMOD_OK) {
//        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
//		exit(-1); // instead of terminate, can change to logging the errors
//    }
//}

//^might have to move this to a utility file if i wanna use in resource manager
//EDIT: moved to AudioUtility.h

// initialize the audio system and load sounds
void AudioHandler::init() {
	FMODErrorCheck(FMOD::System_Create(&system));

	FMODErrorCheck(system->init(36, FMOD_INIT_NORMAL, nullptr));

	ResourceManager::getInstance().init(system); // initialize resource manager with fmod system

	//sounds.resize(static_cast<size_t>(soundID::count), nullptr);
	channels.resize(static_cast<size_t>(soundID::count), nullptr);

	for (int i = 0; i < static_cast<int>(soundID::count); ++i) {
		soundID id = static_cast<soundID>(i);	
		std::string path = soundIDToFilePath(id);

		/*if (id == soundID::bg && !path.empty() && !loadSound(id, path, true)) {
			std::cerr << "Failed to load sound: " << path << std::endl;
		}
		else if (!path.empty() && !loadSound(id, path)) {
			std::cerr << "Failed to load sound: " << path << std::endl;
		}*/

		// loading sounds now handled in ResourceManager
		if (!path.empty()) {
			bool loop = (id == soundID::bg);
			FMOD::Sound* sound = ResourceManager::getInstance().getSound(path, loop);
			if (!sound) {
				std::cerr << "Failed to load sound: " << path << std::endl;
			}
		}

	}
}

// update the audio system each frame and set inactive channel to nullptr
void AudioHandler::update() {
	if (system) {
		system->update();
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]) {
			bool isPlaying = false;
			channels[i]->isPlaying(&isPlaying);  // NEED to call fmoderrorcheck() here once exit(-1) change to logging
			if (!isPlaying) {
				channels[i] = nullptr;
			}
		}
	}

}

// release all sounds and shut down the audio system (will shutdown automatically when dtor called)
void AudioHandler::shutdown() {
	/*for (FMOD::Sound* sound : sounds) {
		if (sound) {
			sound->release();
		}
	}

	sounds.clear();*/
	//ResourceManager will handle releasing sounds now

	channels.clear();

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}
}


/* Sound Management */
// now done in ResourceManager
// load the sound from file path, if loop is true, sound will loop until stopped (default false)
//bool AudioHandler::loadSound(soundID id, const std::string& filePath, bool loop) {
//	if (!system) {
//		std::cerr << "Error! Audio system not initialized..." << std::endl;
//		return false;
//	}
//
//	size_t pos = static_cast<size_t>(id);
//	if (sounds[pos] != nullptr) return true; // means sounds alr loaded, jic called multiple times
//
//	FMOD::Sound* sound = nullptr;
//	FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT; // by default, sound stops after playing once
//	FMODErrorCheck(system->createSound(filePath.c_str(), mode, nullptr, &sound));
//
//	sounds[pos] = sound;
//
//	return true;
//}

// play the sound, return the channel pointer if successful, nullptr if failed
FMOD::Channel* AudioHandler::playSound(soundID id, float volume) {
	if (!system) {
		std::cerr << "Error! Audio system not initialized..." << std::endl;
		return nullptr;
	}

	std::string path = soundIDToFilePath(id);
	if(path.empty()) {
		std::cerr << "Error! Sound file path is empty..." << std::endl;
		return nullptr;
	}
	/*if (sounds[pos] == nullptr) {
		std::cerr << "Error! Sound not loaded..." << std::endl;
		return nullptr;
	}*/
	//bool loop = (id == soundID::bg);
	FMOD::Sound* sound = ResourceManager::getInstance().getSound(path);
	if(!sound) {
		std::cerr << "Error! Sound could not be loaded: " << path << std::endl;
		return nullptr;
	}

	FMOD::Channel* channel = nullptr;
	//FMODErrorCheck(system->playSound(sounds[pos], nullptr, false, &channel));
	FMODErrorCheck(system->playSound(ResourceManager::getInstance().getSound(path), nullptr, false, &channel));
	if (channel) {
		channel->setVolume(volume);
		//channels[pos] = channel; // store this for controlling ltr
		channels[static_cast<size_t>(id)] = channel;
	}

	return channel;
}

// PAUSE / RESUME a particular sound
void AudioHandler::toggleSoundPause(soundID id) {
	size_t pos = static_cast<size_t>(id);
	bool isPaused = false;

	if (channels[pos]) {
		FMODErrorCheck(channels[pos]->getPaused(&isPaused));
		// if paused, resume playing, if playing, pause
		FMODErrorCheck(channels[pos]->setPaused(!isPaused));
	}
}

// pause / resume all sounds
void AudioHandler::toggleAllSoundPause() {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]) {
			bool isPaused = false;
			FMODErrorCheck(channels[i]->getPaused(&isPaused));
			FMODErrorCheck(channels[i]->setPaused(!isPaused));
		}
	}
}

// check whether sound is paused
bool AudioHandler::isSoundPaused(soundID id) {
	size_t pos = static_cast<size_t>(id);
	bool isPaused = false;

	if (channels[pos]) {
		FMODErrorCheck(channels[pos]->getPaused(&isPaused));
		return isPaused;
	}

	return false;
}

// stop a particular sound
void AudioHandler::stopSound(soundID id) {
	size_t pos = static_cast<size_t>(id);

	if (channels[pos]) {
		FMODErrorCheck(channels[pos]->stop());
		channels[pos] = nullptr;
	}
}

// check whether channel still alive (ie. sound finished playing)
bool AudioHandler::isSoundPlaying(soundID id) {
	size_t pos = static_cast<size_t>(id);

	if (channels[pos]) {
		bool isPlaying = false;
		FMODErrorCheck(channels[pos]->isPlaying(&isPlaying));
		return isPlaying;
	}
	return false;
}

// mute specific sound
void AudioHandler::muteSound(soundID id) {
	size_t pos = static_cast<size_t>(id);
	if (channels[pos]) {
		FMODErrorCheck(channels[pos]->setMute(true));
	}
}

// mute all sounds
void AudioHandler::muteAllSound() {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]) {
			FMODErrorCheck(channels[i]->setMute(true));
		}
	}
}

// unmute specific sound
void AudioHandler::unmuteSound(soundID id) {
	size_t pos = static_cast<size_t>(id);
	if (channels[pos]) {
		FMODErrorCheck(channels[pos]->setMute(false));
	}
}

// unmute all sounds
void AudioHandler::unmuteAllSound() {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]) {
			FMODErrorCheck(channels[i]->setMute(false));
		}
	}
}

// increase by 0.1f
void AudioHandler::increaseSoundVolume(soundID id) {
	size_t pos = static_cast<size_t>(id);
	if (channels[pos]) {
		float volume = 0.0f;
		FMODErrorCheck(channels[pos]->getVolume(&volume));
		volume += 0.1f;
		if (volume > 1.0f) volume = 1.0f; // cap at 1.0f
		FMODErrorCheck(channels[pos]->setVolume(volume));
	}
}

// decrease by 0.1f
void AudioHandler::decreaseSoundVolume(soundID id) {
	size_t pos = static_cast<size_t>(id);
	if (channels[pos]) {
		float volume = 0.0f;
		FMODErrorCheck(channels[pos]->getVolume(&volume));
		volume -= 0.1f;
		if (volume < 0.0f) volume = 0.0f; // cap at 0.0f
		FMODErrorCheck(channels[pos]->setVolume(volume));
	}
}

// set volume of a sound to a specific value between 0.0f (muted) to 1.0f (full)
void AudioHandler::setSoundVolume(soundID id, float volume) {
	size_t pos = static_cast<size_t>(id);

	if (channels[pos])
		FMODErrorCheck(channels[pos]->setVolume(volume));
}

// set a specific pitch between 0.5f (half speed) to 2.0f (double speed)
void AudioHandler::setSoundpitch(soundID id, float pitch) {
	size_t pos = static_cast<size_t>(id);
	if (channels[pos])
		FMODErrorCheck(channels[pos]->setPitch(pitch));
}

/* HELPER FUNCTION */
// helper function to convert soundID to file path
std::string AudioHandler::soundIDToFilePath(soundID id) {
	// for now file paths are hardcoded, to change once deserialization is implemented
	switch (id) {
	case soundID::effect:
		return "shaders/effect.wav";
	case soundID::bg:
		return "shaders/bg.wav";
	default:
		return "";
	}
}