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