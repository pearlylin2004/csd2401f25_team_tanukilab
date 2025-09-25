#pragma once

#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <vector>
#include <string>
#include <iostream>


enum class soundID : int {
	water = 0, // for water sound (placeholder only)
	// keep this as the last, for getting the number of sound ids
	count
};

class AudioSystem {
public:
	static AudioSystem& getInstance(); // make sure only 1 instance is created

	void init();
	void update();
	void shutdown();

	static void FMODErrorCheck(FMOD_RESULT result);

private: 
	AudioSystem() : system(nullptr) {}
	~AudioSystem() { shutdown(); } // auto shutdown once instance is destroyed
	// helper function to convert soundID to file path
	// currently hardcoded the path, to change once deserialization is implemented
	std::string soundIDToFilePath(soundID id);
	FMOD::System* system;
	std::vector<FMOD::Sound*> sounds;
	std::vector<FMOD::Channel*> channels;
};