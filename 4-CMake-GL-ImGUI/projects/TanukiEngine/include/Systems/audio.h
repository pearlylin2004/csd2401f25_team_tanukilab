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
#include "Component.h"
#include "GameObjectManager.h"

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

	// play the sound, return the channel pointer if successful, nullptr if failed
	FMOD::Channel* playSound(AudioChannel* audio);

	// call this to PAUSE / RESUME sound
	/*void toggleSoundPause(AudioChannel* audio);*/

	// temp to just pause/resume all audio for game obj
	void pauseAll(GameObjectManager& manager);
	void resumeAll(GameObjectManager& manager);
	void stopAll(GameObjectManager& manager);

	void pauseSound(AudioChannel* audio);

	void resumeSound(AudioChannel* audio);
	// use this to PAUSE / RESUME ALL sounds
	//void toggleAllSoundPause(); // DO NOT USE!!
	// use this to check whether sound is paused
	bool isSoundPaused(AudioChannel* audio);

	// to stop a particular sound
	void stopSound(AudioChannel* audio);
	// to check whether channel still alive (ie. sound finished playing)
	bool isSoundPlaying(AudioChannel* audio);

	// mute specific sound
	void muteSound(AudioChannel* audio);
	// mute all sounds
	//void muteAllSound(); // DO NO USE!!
	// unmute specific sound
	void unmuteSound(AudioChannel* audio);
	// unmute all sounds
	//void unmuteAllSound(); // DO NOT USE!!
	// increase by 0.1f
	void increaseSoundVolume(AudioChannel* audio);
	// decrease by 0.1f
	void decreaseSoundVolume(AudioChannel* audio);
	// set a specific volume
	void setSoundVolume(AudioChannel* audio, float volume); // 0.0f (muted) to 1.0f (full)

	// set a specific pitch
	void setSoundpitch(AudioChannel* audio, float pitch); // 0.5f (half speed) to 2.0f (double speed)

	//pause and resume
	//void pauseAll(); // DO NOT USE!!
	//void resumeAll(); // DO NOT USE!!

	void fadeIn(AudioChannel* audio, float targetVolume = 1.0f, float duration = 1.0f);
	void fadeOut(AudioChannel* audio, float duration = 1.0f);
	bool isFading(AudioChannel* audio) const;
	void stopFade(AudioChannel* audio);

	//setters for fadeing
	void setFadeInDuration(AudioChannel* audio, float duration);
	void setFadeOutDuration(AudioChannel* audio, float duration);
	void setFadeVolume(AudioChannel* audio, float volume);
	//getters
	float getFadeInDuration(AudioChannel* audio) const;
	float getFadeOutDuration(AudioChannel* audio) const;
	float getFadeTargetVolume(AudioChannel* audio) const;
	float getFadeProgress(AudioChannel* audio) const;
	bool isFadingIn(AudioChannel* audio) const;
	bool isFadingOut(AudioChannel* audio) const;

	// Spatial audio
	void updateListener(float listenerX, float listenerY, float listenerZ, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ);

	// Global volume controls (0.0 to 1.0)
	void setMusicVolume(float vol);
	float getMusicVolume() const { return m_musicVolume; }
	void setSFXVolume(float vol);
	float getSFXVolume() const { return m_sfxVolume; }

	// Global volume control (0.0 to 1.0)
	void setMasterVolume(float vol);
	float getMasterVolume() const { return m_masterVolume; }

	void setLowPassFilter(bool enable, float cutoff = 500.f);

	// Register the BGM channel so setMusicVolume can update it live
	void registerBGMChannel(FMOD::Channel* channel) { m_bgmChannel = channel; }

	void requestBGM(const std::string& channelName, GameObjectManager& manager);

	void resetBGM();

	FMOD::Channel* playPreviewSound(AudioChannel* audio);
	void stopAllPreviews();  // stops the entire preview group atomically — no iteration needed

	FMOD::Channel* playBGMSound(AudioChannel* audio);
	// In private, alongside m_gameGroup:
	FMOD::ChannelGroup* m_previewGroup = nullptr;  // editor preview audio lives here
private:
	AudioHandler() : system(nullptr) {}
	~AudioHandler() { shutdown(); }

	FMOD::System* system;
	FMOD::ChannelGroup* m_gameGroup = nullptr;  // all gameplay audio goes here
	FMOD::ChannelGroup* m_bgmGroup = nullptr;

	FMOD::DSP* m_lowPassDSP = nullptr;

	std::string m_currentBGMFile = "";

	float m_musicVolume = 1.0f;
	float m_sfxVolume = 1.0f;
	float m_bgmBaseVolume = 0.125f;
	float m_masterVolume = 1.0f;
	FMOD::Channel* m_bgmChannel = nullptr;


	/* ---- for fade out previous bgm ---- */
	// i stupid lah idk how to do it cleanly
	struct BGMFadeOut {
		FMOD::Channel* channel = nullptr;
		float timer = 0.f;
		float duration = 1.f;
		float startVolume = 0.f;
	};
	std::vector<BGMFadeOut> m_fadingOutBGMs;

};