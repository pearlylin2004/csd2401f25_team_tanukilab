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

// initialize the audio system and load sounds
void AudioHandler::init() {
	FMODErrorCheck(FMOD::System_Create(&system));
	FMODErrorCheck(system->init(36, FMOD_INIT_NORMAL, nullptr));
	FMODErrorCheck(system->set3DSettings(0.0f, 1.0f, 1.0f));

	// Create the game channel group — all gameplay audio routes through here.
	// Editor preview audio plays directly under master and is unaffected by this group's pause state.
	FMODErrorCheck(system->createChannelGroup("game", &m_gameGroup));
	FMODErrorCheck(system->createChannelGroup("preview", &m_previewGroup));
	FMODErrorCheck(system->createChannelGroup("bgm", &m_bgmGroup));

	ResourceManager::getInstance().init(system);
}

// update the audio system each frame and set inactive channel to nullptr
void AudioHandler::update(float deltaTime) {
	(void)deltaTime;
	//record current time for performance tracking
	auto start = std::chrono::high_resolution_clock::now();
	if (system) {
		system->update();
	}

	// fade out previous bgm manually
	for (auto it = m_fadingOutBGMs.begin(); it != m_fadingOutBGMs.end();) {
		it->timer += deltaTime;
		float progress = std::min(it->timer / it->duration, 1.f);
		it->channel->setVolume(it->startVolume * (1.f - progress));

		if (progress >= 1.f) {
			it->channel->stop();
			it = m_fadingOutBGMs.erase(it);
		}
		else {
			++it;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	double ms = std::chrono::duration<double, std::milli>(end - start).count(); //get elapse time in ms
	g_SystemTimers.push_back({ "Audio", ms }); //saving timing for UI output
}

// release all sounds and shut down the audio system (will shutdown automatically when dtor called)
void AudioHandler::shutdown() {

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}
}

// play the sound, return the channel pointer if successful, nullptr if failed
FMOD::Channel* AudioHandler::playSound(AudioChannel* audio) {
	if (!system) {
		std::cerr << "Error! Audio system not initialized..." << std::endl;
		return nullptr;
	}

	if (audio->audioFile.empty()) {
		std::cerr << "Error! Sound file path is empty..." << std::endl;
		return nullptr;
	}

	// Pass isSpatial flag from AudioChannel to ResourceManager
	FMOD::Sound* sound = ResourceManager::getInstance().getSound(
		audio->audioFile,
		audio->loop,
		audio->isSpatial  // ← This is the key change!
	);

	if (!sound) {
		std::cerr << "Error! Sound could not be loaded: " << audio->audioFile << std::endl;
		return nullptr;
	}

	// For 3D sounds, override default min/max distance with channel-specific values

	FMOD::Channel* channel = nullptr;
	FMODErrorCheck(system->playSound(sound, m_gameGroup, false, &channel));

	if (channel) {
		channel->setVolume(audio->volume);
		channel->setPitch(audio->pitch);
		//channel->setMode(audio->loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

		if (audio->isSpatial) {
			FMOD_RESULT distResult = channel->set3DMinMaxDistance(
				audio->minDistance,
				audio->maxDistance
			);
			if (distResult != FMOD_OK) {
				std::cerr << "Warning: Failed to set 3D distance for: " << audio->audioFile << std::endl;
			}

			// Spread: 0 = point source (hard pan), 360 = full both ears.
			// 180 means you still hear it from both ears, but it's more directional than the default 360.
			channel->set3DSpread(audio->spread);
		}
		audio->channel = channel;
		audio->fadeInfo.isFading = false;
		//std::cout << "[AUDIO] Playing: " << audio->audioFile << (audio->isSpatial ? " (3D)" : " (2D)") << std::endl;
	}

	return channel;
}

void AudioHandler::pauseAll(GameObjectManager& manager) {
	// Pause the entire game group — catches ghost channels (e.g. menu BGM)
	// that no longer have a live GameObject.
	if (m_gameGroup) m_gameGroup->setPaused(true);
	if (m_bgmGroup) m_bgmGroup->setPaused(true);


	// Sync AudioState so the component layer stays accurate.
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();
		for (auto& pair : ac->audioChannels) {
			AudioChannel& ch = pair.second;
			if (ch.state == AudioState::Playing && ch.channel)
				ch.state = AudioState::Paused;  // FMOD already paused by group above
		}
	}
}

void AudioHandler::resumeAll(GameObjectManager& manager) {
	if (m_gameGroup) m_gameGroup->setPaused(false);
	if (m_bgmGroup) m_bgmGroup->setPaused(false);

	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();
		for (auto& pair : ac->audioChannels) {
			AudioChannel& ch = pair.second;
			if (ch.state == AudioState::Paused && ch.channel)
				ch.state = AudioState::Playing;  // FMOD already resumed by group above
		}
	}
}

void AudioHandler::stopAll(GameObjectManager& manager) {
	std::vector<GameObject*> gameObjects;
	manager.getAllGameObjects(gameObjects);

	// Before stopping the game group, rescue any persist-flagged channels
	// by moving them into the BGM group so they keep playing through the scene change.
	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();
		for (auto& [name, ch] : ac->audioChannels) {
			if (ch.persistOnSceneChange && ch.channel && m_bgmGroup) {
				ch.channel->setChannelGroup(m_bgmGroup);
			}
		}
	}

	// Stop the entire game group to catch all non-persist and orphaned FMOD channels.
	if (m_gameGroup) m_gameGroup->stop();

	// Reset channel state, but leave persist channels alone so their FMOD channel stays valid.
	for (GameObject* obj : gameObjects) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();
		for (auto& [name, ch] : ac->audioChannels) {
			if (!ch.persistOnSceneChange) {
				ch.channel = nullptr;
				ch.state = AudioState::Stopped;
				ch.isPendingPlay = false;
				ch.isPendingStop = false;
			}
		}
	}
}

// PAUSE sound
void AudioHandler::pauseSound(AudioChannel* audio) {
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setPaused(true));
	}
}

// RESUME sound
void AudioHandler::resumeSound(AudioChannel* audio) {
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setPaused(false));
	}
}

// check whether sound is paused
bool AudioHandler::isSoundPaused(AudioChannel* audio) {
	bool isPaused = false;
	if (audio->channel) {
		FMOD_RESULT result = audio->channel->getPaused(&isPaused);
		//if sound finished naturally, the channel handle becomes invalid, so treat it as not paused and clean up the handle
		if (result == FMOD_ERR_INVALID_HANDLE) {
			audio->channel = nullptr;
			return false;
		}
		FMODErrorCheck(result);
		return isPaused;
	}
	return false;
}

// stop a particular sound
void AudioHandler::stopSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->stop());
		audio->channel = nullptr;
		audio->fadeInfo.isFading = false;
	}
}

// check whether channel still alive (ie. sound finished playing)
bool AudioHandler::isSoundPlaying(AudioChannel* audio) {
	if (audio->channel) {
		bool isPlaying = false;
		FMOD_RESULT result = audio->channel->isPlaying(&isPlaying);
		if (result == FMOD_ERR_INVALID_HANDLE) {
			// Channel expired naturally (sound finished) — clean up silently
			audio->channel = nullptr;
			return false;
		}
		FMODErrorCheck(result); // still report genuinely unexpected errors
		return isPlaying;
	}
	return false;
}

// mute specific sound
void AudioHandler::muteSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setMute(true));
	}
}

// unmute specific sound
void AudioHandler::unmuteSound(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);
	/*if (channels[pos]) {*/
	if (audio->channel) {
		FMODErrorCheck(audio->channel->setMute(false));
	}
}

// increase by 0.1f
void AudioHandler::increaseSoundVolume(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel) {
		float volume = 0.0f;
		FMODErrorCheck(audio->channel->getVolume(&volume));
		volume += 0.1f;
		if (volume > 1.0f) volume = 1.0f; // cap at 1.0f
		FMODErrorCheck(audio->channel->setVolume(volume));
	}
}

// decrease by 0.1f
void AudioHandler::decreaseSoundVolume(AudioChannel* audio) {
	//size_t pos = static_cast<size_t>(id);

	if (audio->channel) {
		float volume = 0.0f;
		FMODErrorCheck(audio->channel->getVolume(&volume));
		volume -= 0.1f;
		if (volume < 0.0f) volume = 0.0f; // cap at 0.0f
		FMODErrorCheck(audio->channel->setVolume(volume));
	}
}

// set volume of a sound to a specific value between 0.0f (muted) to 1.0f (full)
void AudioHandler::setSoundVolume(AudioChannel* audio, float volume) {
	if (audio->channel)
		FMODErrorCheck(audio->channel->setVolume(volume));
}

// set a specific pitch between 0.5f (half speed) to 2.0f (double speed)
void AudioHandler::setSoundpitch(AudioChannel* audio, float pitch) {
	if (audio->channel)
		FMODErrorCheck(audio->channel->setPitch(pitch));
}

void AudioHandler::fadeIn(AudioChannel* audio, float targetVolume, float duration) {
	if (!audio->channel) {
		std::cerr << "Error! Cannot fade in - channel not active..." << std::endl;
		return;
	}

	FMODErrorCheck(audio->channel->setVolume(0.0f));

	audio->fadeInfo.isFading = true;
	audio->fadeInfo.isFadingIn = true;
	audio->fadeInfo.startVolume = 0.0f;
	audio->fadeInfo.targetVolume = targetVolume;
	audio->fadeInfo.fadeDuration = duration;
	audio->fadeInfo.fadeTimer = 0.0f;
}

void AudioHandler::fadeOut(AudioChannel* audio, float duration) {
	//size_t pos = static_cast<size_t>(id);

	if (!audio->channel) {
		std::cerr << "Error! Cannot fade out - channel not active..." << std::endl;
		return;
	}

	float currentVolume = 0.0f;
	FMODErrorCheck(audio->channel->getVolume(&currentVolume));

	audio->fadeInfo.isFading = true;
	audio->fadeInfo.isFadingIn = false;
	audio->fadeInfo.startVolume = currentVolume;
	audio->fadeInfo.targetVolume = 0.f;
	audio->fadeInfo.fadeDuration = duration;
	audio->fadeInfo.fadeTimer = 0.f;
}

// check if a sound is  fading
bool AudioHandler::isFading(AudioChannel* audio) const {
	return audio->fadeInfo.isFading;
}

// stop any active fade for a sound
void AudioHandler::stopFade(AudioChannel* audio) {
	audio->fadeInfo.isFading = false;
}

// Setters for audio fading
void AudioHandler::setFadeInDuration(AudioChannel* audio, float duration) {

	//if (pos < fadeInfo.size() && duration > 0.0f) {
	audio->fadeInfo.defaultFadeInDuration = duration;
	//}
}

void AudioHandler::setFadeOutDuration(AudioChannel* audio, float duration) {

	//if (pos < fadeInfo.size() && duration > 0.0f) {
	audio->fadeInfo.defaultFadeOutDuration = duration;
	//}
}

void AudioHandler::setFadeVolume(AudioChannel* audio, float volume) {
	audio->fadeInfo.targetVolume = std::max(0.0f, std::min(volume, 1.0f)); // clamp between 0 and 1
	//}
}

// Getters
float AudioHandler::getFadeInDuration(AudioChannel* audio) const {
	return audio->fadeInfo.defaultFadeInDuration;
}

float AudioHandler::getFadeOutDuration(AudioChannel* audio) const {
	return audio->fadeInfo.defaultFadeOutDuration;
}

float AudioHandler::getFadeTargetVolume(AudioChannel* audio) const {
	return audio->fadeInfo.targetVolume;
}

float AudioHandler::getFadeProgress(AudioChannel* audio) const {
	if (audio->fadeInfo.isFading && audio->fadeInfo.fadeDuration > 0.0f) {
		return audio->fadeInfo.fadeTimer / audio->fadeInfo.fadeDuration;
	}
	return 0.0f;
}

bool AudioHandler::isFadingIn(AudioChannel* audio) const {
	return audio->fadeInfo.isFading && audio->fadeInfo.isFadingIn;
}

bool AudioHandler::isFadingOut(AudioChannel* audio) const {
	return audio->fadeInfo.isFading && !audio->fadeInfo.isFadingIn;
}

void AudioHandler::updateListener(float listenerX, float listenerY, float listenerZ,
	float forwardX, float forwardY, float forwardZ,
	float upX, float upY, float upZ) {
	if (!system) return;

	// Set listener position
	FMOD_VECTOR position = { listenerX, listenerY, listenerZ };

	// Set listener velocity (velocity = 0 means no doppler effect)
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

	// Set listener orientation
	FMOD_VECTOR forward = { forwardX, forwardY, forwardZ };
	FMOD_VECTOR up = { upX, upY, upZ };

	// Tell FMOD about the listener
	FMODErrorCheck(system->set3DListenerAttributes(
		0,           // Listener index (0 = first/only listener)
		&position,
		&velocity,
		&forward,
		&up
	));
}

void AudioHandler::setMusicVolume(float vol) {
	m_musicVolume = std::max(0.0f, std::min(vol, 1.0f));
	if (m_bgmChannel) {
		m_bgmChannel->setVolume(m_bgmBaseVolume * m_musicVolume);
	}
}

void AudioHandler::setSFXVolume(float vol) {
	m_sfxVolume = std::max(0.0f, std::min(vol, 1.0f));
}

void AudioHandler::setMasterVolume(float vol) {
    m_masterVolume = std::max(0.0f, std::min(vol, 1.0f));

    // FMOD's master channel group controls ALL active channels at once
    FMOD::ChannelGroup* masterGroup = nullptr;
    if (system) {
        system->getMasterChannelGroup(&masterGroup);
        if (masterGroup) {
            masterGroup->setVolume(m_masterVolume);
        }
    }
}

void AudioHandler::setLowPassFilter(bool enable, float cutoff) {
	if (!system) return;

	if (enable) {
		if (!m_lowPassDSP) {
			system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &m_lowPassDSP);
		}

		// add to master channel group
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			masterGroup->addDSP(0, m_lowPassDSP);
			m_lowPassDSP->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoff);
			m_lowPassDSP->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, 1.f);
			m_lowPassDSP->setActive(true);
		}
	}
	else {
		if (m_lowPassDSP) {
			FMOD::ChannelGroup* masterGroup = nullptr;

			system->getMasterChannelGroup(&masterGroup);
			if (masterGroup)
				masterGroup->removeDSP(m_lowPassDSP);

			m_lowPassDSP->release();
			m_lowPassDSP = nullptr;
		}
	}
}

void AudioHandler::requestBGM(const std::string& channelName, GameObjectManager& manager) {
	// find the object that has this channel
	std::vector<GameObject*> objs;
	manager.getAllGameObjects(objs);


	for (GameObject* obj : objs) {
		if (!obj->hasComponent<AudioComponent>()) continue;
		AudioComponent* ac = obj->getComponent<AudioComponent>();
		if (!ac->hasChannel(channelName)) continue;
		AudioChannel* ch = ac->getOrCreateChannel(channelName);

		// skip if same bgm requested
		if (ch->audioFile == m_currentBGMFile) return;

		if (m_bgmChannel) {
			float vol = 0.f;
			m_bgmChannel->getVolume(&vol);
			float dur = ch->fadeOutDuration > 0.f ? ch->fadeOutDuration : 1.f;
			m_fadingOutBGMs.push_back({ m_bgmChannel, 0.f, dur, vol });
			m_bgmChannel = nullptr;
		}

		//resetBGM();

		// play new bgm under bgm group
		m_bgmChannel = playBGMSound(ch);
		m_currentBGMFile = ch->audioFile;
		return;
	}
}

void AudioHandler::resetBGM() {
	if (m_bgmChannel) {
		m_bgmChannel->stop();
		m_bgmChannel = nullptr;
	}

	m_currentBGMFile.clear();
}

FMOD::Channel* AudioHandler::playBGMSound(AudioChannel* audio) {
	FMOD::Sound* sound = ResourceManager::getInstance().getSound(audio->audioFile, audio->loop, false);
	if (!sound) return nullptr;
	FMOD::Channel* channel = nullptr;
	FMODErrorCheck(system->playSound(sound, m_bgmGroup, false, &channel));

	if (channel) {
		audio->channel = channel;
		if (audio->fadeInOnStart && audio->fadeInDuration > 0.f) {
			channel->setVolume(0.f);
			fadeIn(audio, audio->volume, audio->fadeInDuration);
		}
		else {
			channel->setVolume(audio->volume);
		}
	}
	return channel;
}

FMOD::Channel* AudioHandler::playPreviewSound(AudioChannel* audio) {
	if (!system || audio->audioFile.empty()) return nullptr;

	// Stop whatever was playing on this channel before starting a new preview.
	// This handles: clicking Play twice, changing the audio file, switching selected objects.
	if (audio->channel) {
		audio->channel->stop();
		audio->channel = nullptr;
		audio->state = AudioState::Stopped;
	}

	FMOD::Sound* sound = ResourceManager::getInstance().getSound(
		audio->audioFile, audio->loop, audio->isSpatial
	);
	if (!sound) return nullptr;

	FMOD::Channel* channel = nullptr;
	FMODErrorCheck(system->playSound(sound, m_previewGroup, false, &channel));

	if (channel) {
		channel->setVolume(audio->volume);
		channel->setPitch(audio->pitch);
		audio->channel = channel;
		audio->fadeInfo.isFading = false;
	}

	return channel;
}

void AudioHandler::stopAllPreviews() {
	if (m_previewGroup) {
		m_previewGroup->stop();  // atomically kills every preview channel
	}
}