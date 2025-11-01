#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>

#include "GL/glew.h"
#include <../lib/FMOD/fmod.hpp>
#include "stb_image.h"
#include "shader.hpp"
#include "AudioUtility.h"

struct TextureData {
    GLuint id = 0;
    bool isTransparent = false;
};

class ResourceManager {
public:
    /**
     * @brief Gets the singleton instance of the ResourceManager.
     */
    static ResourceManager& getInstance();

    /**
     * @brief Initializes the manager. Call this once at startup.
     * @param fmodSystem A pointer to the FMOD system from AudioHandler.
     */
    void init(FMOD::System* fmodSystem);

    /**
     * @brief Cleans up and releases all loaded resources. Call this at shutdown.
     */
    void shutdown();

    /**
     * @brief Gets a texture. Loads it from file if not in cache.
     * @param path The filepath to the texture.
     * @return A struct containing the texture ID and transparency info.
     */
    TextureData getTexture(const std::string& path);

    /**
     * @brief Gets a sound. Loads it from file if not in cache.
     * @param path The filepath to the sound.
     * @param loop Whether the sound should loop (FMOD_LOOP_NORMAL).
     * @return A pointer to the FMOD::Sound object.
     */
    FMOD::Sound* getSound(const std::string& path, bool loop = false);

    /**
     * @brief Gets a shader program. Loads and links it if not in cache.
     * @param vertPath The filepath to the vertex shader.
     * @param fragPath The filepath to the fragment shader.
     * @return The GLuint program ID.
     */
    GLuint getShader(const std::string& vertPath, const std::string& fragPath);

    // Delete copy/move constructors and assignment operators
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

private:
    ResourceManager() = default;  // Private constructor
    ~ResourceManager() = default; // Private destructor

    // Private helper functions to load from disk
    TextureData loadTextureFromFile(const std::string& path);
    FMOD::Sound* loadSoundFromFile(const std::string& path, bool loop);

    // FMOD system instance (required for sound creation)
    FMOD::System* m_fmodSystem = nullptr;

    // Caches for loaded resources
    std::unordered_map<std::string, TextureData> m_textureCache;
    std::unordered_map<std::string, FMOD::Sound*> m_audioCache;
    std::unordered_map<std::string, GLuint> m_shaderCache;

};
