#include "ResourceManager.h"
/*
ResourceManager& ResourceManager::getInstance() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::init(FMOD::System* fmodSystem) {
    m_fmodSystem = fmodSystem;
}

void ResourceManager::shutdown() {

    //Release all textures
    for (auto& pair : m_textureCache) {
        glDeleteTextures(1, &pair.second.id);
    }
    m_textureCache.clear();
    std::cout << "ResourceManager: Cleared all textures." << std::endl;

    //Release all sounds
    for (auto& pair : m_audioCache) {
        pair.second->release();
    }
    m_audioCache.clear();
    std::cout << "ResourceManager: Cleared all sounds." << std::endl;

    //Release all shader programs
    for (auto& pair : m_shaderCache) {
        glDeleteProgram(pair.second);
    }
    m_shaderCache.clear();
    std::cout << "ResourceManager: Cleared all shader programs." << std::endl;
}

TextureData ResourceManager::getTexture(const std::string& path) {
    if (path.empty()) {
        return { 0, false };
    }

    //Check if texture is already in cache
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    //Not in cache, load it from file
    std::cout << "ResourceManager: Loading texture: " << path << std::endl;
    TextureData newTexture = loadTextureFromFile(path);

    if (newTexture.id != 0) {
        m_textureCache[path] = newTexture;
    }

    return newTexture;
}

TextureData ResourceManager::loadTextureFromFile(const std::string& filename) {
	//From RenderSystem::uploadtex
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return {0, false};
    }

    std::cout << "Loaded " << filename
        << " (" << width << "x" << height << "), "
        << channels << " channels" << std::endl;

	TextureData textureData;
    textureData.isTransparent = (channels == 4);

    GLuint texobj_hdl;
    glCreateTextures(GL_TEXTURE_2D, 1, &texobj_hdl);
    glTextureStorage2D(texobj_hdl, 1, GL_RGBA8, static_cast<GLuint>(width), static_cast<GLuint>(height));
    glTextureSubImage2D(texobj_hdl, 0, 0, 0, static_cast<GLuint>(width), static_cast<GLuint>(height), GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTextureParameteri(texobj_hdl, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texobj_hdl, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texobj_hdl, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(data);
    return textureData;
}

FMOD::Sound* ResourceManager::getSound(const std::string& path, bool loop) {
    if (path.empty() || !m_fmodSystem) {
        return nullptr;
    }

    auto it = m_audioCache.find(path);
    if (it != m_audioCache.end()) {
        return it->second;
    }

    // Not in cache, load it
    std::cout << "ResourceManager: Loading sound: " << path << std::endl;
    FMOD::Sound* newSound = loadSoundFromFile(path, loop);

    if (newSound) {
        m_audioCache[path] = newSound;
    }

    return newSound;
}

FMOD::Sound* ResourceManager::loadSoundFromFile(const std::string& path, bool loop) {
    // This logic is moved from AudioHandler::loadSound
    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    FMOD_RESULT result = m_fmodSystem->createSound(path.c_str(), mode, nullptr, &sound);
    AudioHandler::FMODErrorCheck(result); // Use the static error checker

    if (result != FMOD_OK) {
        std::cerr << "ResourceManager Error: Failed to create sound: " << path << std::endl;
        return nullptr;
    }

    return sound;
}

GLuint ResourceManager::getShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    if (vertPath.empty() || fragPath.empty()) {
        return 0;
    }

    // Create a unique key for the cache
    std::string key = vertPath + fragPath;

    // Check if shader is already in cache
    auto it = m_shaderCache.find(key);
    if (it != m_shaderCache.end()) {
        return it->second;
    }

    // Not in cache, load it by calling the existing LoadShaders function
    std::cout << "ResourceManager: Loading shader program: " << vertPath << " + " << fragPath << std::endl;

    // We call the existing global LoadShaders function from shader.cpp
    GLuint newProgramID = LoadShaders(vertPath, fragPath, true);

    if (newProgramID != 0) {
        m_shaderCache[key] = newProgramID;
    }

    return newProgramID;
}

*/