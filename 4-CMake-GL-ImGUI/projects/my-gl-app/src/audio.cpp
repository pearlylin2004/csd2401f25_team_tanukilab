#include "audio.h"

AudioSystem& AudioSystem::getInstance() {
	static AudioSystem instance;
	return instance;
}

void AudioSystem::FMODErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD error: " << FMOD_ErrorString(result) << std::endl;
		exit(-1); // instead of terminate, can change to logging the errors
    }
}

void AudioSystem::init() {
	FMODErrorCheck(FMOD::System_Create(&system));

	FMODErrorCheck(system->init(36, FMOD_INIT_NORMAL, nullptr));

	sounds.resize(static_cast<size_t>(soundID::count), nullptr);
	channels.resize(static_cast<size_t>(soundID::count), nullptr);

	for (int i = 0; i < static_cast<int>(soundID::count); ++i) {
		soundID id = static_cast<soundID>(i);
		std::string path = soundIDToFilePath(id);

		if (!path.empty() && !loadSound(id, path)) {
			std::cerr << "Failed to load sound: " << path << std::endl;
		}
	}
}

void AudioSystem::update() {
	if (system) {
		system->update();
	}
}

void AudioSystem::shutdown() {
	for (FMOD::Sound* sound : sounds) {
		if (sound) {
			sound->release();
		}
	}

	sounds.clear();
	channels.clear();

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}
}


/* Sound Management */
bool AudioSystem::loadSound(soundID id, const std::string& filePath, bool loop) {
	if (!system) {
		std::cerr << "Error! Audio system not initialized..." << std::endl;
		return false;
	}

	size_t pos = static_cast<size_t>(id);
	if (sounds[pos] != nullptr) return true; // means sounds alr loaded, jic called multiple times

	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT; // by default, sound stops after playing once
	FMODErrorCheck(system->createSound(filePath.c_str(), mode, nullptr, &sound));

	sounds[pos] = sound;

	return true;
}

FMOD::Channel* AudioSystem::playSound(soundID id, float volume) {
	if (!system) {
		std::cerr << "Error! Audio system not initialized..." << std::endl;
		return nullptr;
	}

	size_t pos = static_cast<size_t>(id);
	if (sounds[pos] == nullptr) {
		std::cerr << "Error! Sound not loaded..." << std::endl;
		return nullptr;
	}

	FMOD::Channel* channel = nullptr;
	FMODErrorCheck(system->playSound(sounds[pos], nullptr, false, &channel));
	if (channel) {
		channel->setVolume(volume);
		channels[pos] = channel; // store this for controlling ltr
	}

	return channel;
}
/* HELPER FUNCTION */
std::string AudioSystem::soundIDToFilePath(soundID id) {
	// for now file paths are hardcoded, to change once deserialization is implemented
	switch (id) {
	case soundID::water:
		return "shaders/water.mp3";
	default:
		return "";
	}
}