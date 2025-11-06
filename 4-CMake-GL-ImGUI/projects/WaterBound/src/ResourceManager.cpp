#include "ResourceManager.h"

class AudioHandler; // Forward declaration

ResourceManager& ResourceManager::getInstance() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::init(FMOD::System* fmodSystem) {
    m_fmodSystem = fmodSystem;

    if(FT_Init_FreeType(&m_ftLibrary)) {
        std::cerr << "ResourceManager Error: Could not init FreeType Library" << std::endl;
        return;
    }

    std::cout << "ResourceManager: Initialized with FMOD and FreeType." << std::endl;
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
    std::cout << "ResourceManager: Cleared all shaders." << std::endl;

    //Release all fonts and their character textures
    for (auto& pair : m_fontCache) {
        //Delete all character textures for this font
        for (auto& charPair : pair.second.characters) {
            if (charPair.second.TextureID) {
                glDeleteTextures(1, &charPair.second.TextureID);
            }
        }
        // Free the FreeType face
        FT_Done_Face(pair.second.face);
    }
    m_fontCache.clear();
    std::cout << "ResourceManager: Cleared all fonts and character textures." << std::endl;

    if(m_ftLibrary) {
        FT_Done_FreeType(m_ftLibrary);
        m_ftLibrary = nullptr;
        std::cout << "ResourceManager: FreeType library closed." << std::endl;
	}

	std::cout << "ResourceManager: Shutdown complete." << std::endl;
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
        std::cout << "ResourceManager: Successfully cached texture with ID: " << newTexture.id << std::endl;
    }
    else {
		std::cout << "ResourceManager Error: Failed to load texture: " << path << std::endl;
    }

    return newTexture;
}

TextureData ResourceManager::loadTextureFromFile(const std::string& filename) {
	stbi_set_flip_vertically_on_load(true);
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
    
    textureData.id = texobj_hdl;
    
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
    }else{
		std::cout << "ResourceManager Error: Failed to load sound: " << path << std::endl;
    }

    return newSound;
}

FMOD::Sound* ResourceManager::loadSoundFromFile(const std::string& path, bool loop) {
    //Taken from AudioHandler::loadSound
    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    FMOD_RESULT result = m_fmodSystem->createSound(path.c_str(), mode, nullptr, &sound);
    FMODErrorCheck(result); 

    if (result != FMOD_OK) {
        std::cerr << "ResourceManager Error: Failed to create sound: " << path << std::endl;
        return nullptr;
    }
        
    return sound;
}

GLuint ResourceManager::getShader(const std::string& vertPath, const std::string& fragPath) {
    if (vertPath.empty() || fragPath.empty()) {
        return 0;
    }

    // Create key for the cache
    std::string key = vertPath + "|" + fragPath;

    //Check if shader is already in cache
    auto it = m_shaderCache.find(key);
    if (it != m_shaderCache.end()) {
        return it->second;
    }

    //If not in cache, load it by calling the existing LoadShaders function
    std::cout << "ResourceManager: Loading shader program: " << vertPath << " + " << fragPath << std::endl;

    //Call the existing global LoadShaders function from shader.cpp
    GLuint newProgramID = LoadShaders(vertPath, fragPath, true);

    if (newProgramID != 0) {
        m_shaderCache[key] = newProgramID;
    }else{
		std::cout << "ResourceManager Error: Failed to load shader program: " << vertPath << " + " << fragPath << std::endl;
    }

    return newProgramID;
}

const FontData& ResourceManager::getFont(const std::string& relativePath) {
    if(relativePath.empty()) {
		throw std::runtime_error("ResourceManager Error: Font path is empty.");
	}

    //check cache
    auto it = m_fontCache.find(relativePath);
    if (it != m_fontCache.end()) {
        return it->second;
    }

    //not in cache, load it
    std::cout << "ResourceManager: Loading font: " << relativePath << std::endl;
    FontData newFont = loadFontFromFile(relativePath);

    std::cout << "New font has " << newFont.characters.size() << " characters" << std::endl;

    if (newFont.face) {
        m_fontCache[relativePath] = newFont;
        std::cout << "ResourceManager: Successfully cached font: " << relativePath << std::endl;
    }
    else {
        std::cout << "ResourceManager Error: Failed to load font: " << relativePath << std::endl;
    }
    return m_fontCache[relativePath];
}

FontData ResourceManager::loadFontFromFile(const std::string& path) {
    FontData fontData;

    if (!m_ftLibrary) {
        std::cerr << "ResourceManager Error: FreeType library not initialized." << std::endl;
        return fontData;
    }

    if (FT_New_Face(m_ftLibrary, path.c_str(), 0, &fontData.face)) {
        std::cerr << "ResourceManager Error: Failed to load font face: " << path << std::endl;
        return fontData;
    }

    FT_Set_Pixel_Sizes(fontData.face, 0, 48); //set default size

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 255; c++) {
        // load character glyph 
        if (FT_Load_Char(fontData.face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            fontData.face->glyph->bitmap.width,
            fontData.face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            fontData.face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        FontCharacter character = {
            texture,
            glm::ivec2(fontData.face->glyph->bitmap.width, fontData.face->glyph->bitmap.rows),
            glm::ivec2(fontData.face->glyph->bitmap_left, fontData.face->glyph->bitmap_top),
            fontData.face->glyph->advance.x
        };

        fontData.characters[c] = character;
    }

	fontData.isLoaded = true;
    std::cout << "Loaded font: " << path << " (" << fontData.characters.size() << " characters)" << std::endl;

    return fontData;
}

std::vector<std::string> ResourceManager::getLoadedFontPaths() const {
	std::vector<std::string> paths;
    for (const auto& pair : m_fontCache) {
        paths.push_back(pair.first);
	}
	return paths;
}
