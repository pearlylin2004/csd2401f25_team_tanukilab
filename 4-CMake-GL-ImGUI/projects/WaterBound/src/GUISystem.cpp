#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include <iostream>
#include <fstream>

#define RUNTIME_SCENE_DIR_R "./shaders/Scene"

static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}

GUISystem::GUISystem() {
    std::cout << "GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager) {
    std::cout << "GUISystem::init started" << std::endl;

    // Load button data from JSON first - use the same naming convention as level files
    if (!loadButtonDataFromJSON("menu_scene.json")) {
        std::cout << "Failed to load button data from JSON" << std::endl;
    }

    createMenuButtons(manager);
    //std::cout << "GUISystem::init completed successfully" << std::endl;
}

bool GUISystem::loadButtonDataFromJSON(const std::string& filename) {
    std::cout << "Loading button data from: " << filename << std::endl;

    // Use the same path resolution as GameObjectManager
    std::string fullPath = runtimeScenePath(filename);
    //std::cout << "Full path: " << fullPath << std::endl;

    rapidjson::Document doc;
    std::string err;

    // Use the same JsonIO utility that GameObjectManager uses
    if (!JsonIO::ReadFileToDocument(fullPath, doc, &err)) {
        std::cout << "ERROR: Could not load JSON from " << fullPath << ": " << err << std::endl;

        // Try without path resolution as fallback
        if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
            std::cout << "ERROR: Could not load JSON from " << filename << " either: " << err << std::endl;
            return false;
        }
    }

    if (!doc.HasMember("buttons") || !doc["buttons"].IsArray()) {
        std::cout << "ERROR: JSON missing 'buttons' array" << std::endl;
        return false;
    }

    const auto& buttons = doc["buttons"].GetArray();
    m_buttonData.clear();

    for (const auto& button : buttons) {
        if (!button.IsObject() || !button.HasMember("name") || !button.HasMember("components")) {
            std::cout << "WARNING: Skipping invalid button object" << std::endl;
            continue;
        }

        std::string name = button["name"].GetString();
        const auto& components = button["components"];

        if (!components.HasMember("Transform") || !components["Transform"].IsObject()) {
            std::cout << "WARNING: Button '" << name << "' missing Transform component" << std::endl;
            continue;
        }

        const auto& transform = components["Transform"];
        ButtonData data;

        // Load position
        if (transform.HasMember("pos") && transform["pos"].IsArray() && transform["pos"].Size() == 3) {
            data.position.x = transform["pos"][0].GetFloat();
            data.position.y = transform["pos"][1].GetFloat();
            data.position.z = transform["pos"][2].GetFloat();
        }

        // Load scale
        if (transform.HasMember("scale") && transform["scale"].IsArray() && transform["scale"].Size() == 3) {
            data.scale.x = transform["scale"][0].GetFloat();
            data.scale.y = transform["scale"][1].GetFloat();
            data.scale.z = transform["scale"][2].GetFloat();
        }

        m_buttonData[name] = data;
        std::cout << "Loaded button: " << name
            << " pos: (" << data.position.x << ", " << data.position.y << ", " << data.position.z
            << ") scale: (" << data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
    }

    m_buttonDataLoaded = true;
    std::cout << "Successfully loaded " << m_buttonData.size() << " buttons from JSON" << std::endl;
    return true;
}

void GUISystem::update(GameObjectManager& manager) {
    // Only handle GUI interactions in menu state
    if (m_currentState == GameState::MENU) {
        // Check for button clicks
        if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
            // Get mouse position as Vector2D
            Vector2D mousePos = InputHandler::getMousePosition();

            // Convert screen coordinates to world coordinates
            Vector2D worldMousePos = InputHandler::getWorldMousePosition();
            glm::vec2 worldPos = glm::vec2(worldMousePos.x, worldMousePos.y);

            // Check play button click
            if (m_playButton && isPointInButton(worldPos, m_playButton)) {
                startGame(manager);
                return;
            }

            // Check stop button click  
            if (m_stopButton && isPointInButton(worldPos, m_stopButton)) {
                stopGame(manager);
                return;
            }
        }
    }
}

void GUISystem::createMenuButtons(GameObjectManager& manager) {
    //std::cout << "GUISystem::createMenuButtons started" << std::endl;

    if (m_buttonsCreated) {
        //std::cout << "Buttons already created, returning early" << std::endl;
        return;
    }

    std::cout << "Creating buttons..." << std::endl;

    // Create Play Button using JSON data
    m_playButton = manager.createGameObject("play_button");
    if (!m_playButton) {
        std::cout << "ERROR: Failed to create play button!" << std::endl;
        return;
    }

    Render* playRender = m_playButton->addComponent<Render>();
    playRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    playRender->hasTex = false;

    Transform* playTransform = m_playButton->addComponent<Transform>();

    // Use JSON data if available, otherwise use defaults
    if (m_buttonDataLoaded && m_buttonData.find("play_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["play_button"];
        playTransform->x = data.position.x;
        playTransform->y = data.position.y;
        playTransform->z = data.position.z;
        playTransform->scaleX = data.scale.x;
        playTransform->scaleY = data.scale.y;
        playTransform->scaleZ = data.scale.z;
        std::cout << "Using JSON data for play button" << std::endl;
    }
    //else {
    //    // Fallback to hardcoded values
    //    playTransform->x = -6.0f;
    //    playTransform->y = 0.0f;
    //    playTransform->z = 0.1f;
    //    playTransform->scaleX = 6.0f;
    //    playTransform->scaleY = 1.0f;
    //    playTransform->scaleZ = 1.0f;
    //    std::cout << "Using default data for play button" << std::endl;
    //}

    m_stopButton = manager.createGameObject("stop_button");
    if (!m_stopButton) {
        std::cout << "ERROR: Failed to create stop button!" << std::endl;
        return;
    }

    Render* stopRender = m_stopButton->addComponent<Render>();
    stopRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    stopRender->hasTex = false;

    Transform* stopTransform = m_stopButton->addComponent<Transform>();

    if (m_buttonDataLoaded && m_buttonData.find("stop_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["stop_button"];
        stopTransform->x = data.position.x;
        stopTransform->y = data.position.y;
        stopTransform->z = data.position.z;
        stopTransform->scaleX = data.scale.x;
        stopTransform->scaleY = data.scale.y;
        stopTransform->scaleZ = data.scale.z;
        std::cout << "Using JSON data for stop button" << std::endl;
    }
    //else {
    //    // Fallback to hardcoded values
    //    stopTransform->x = 3.0f;
    //    stopTransform->y = 0.0f;
    //    stopTransform->z = 0.1f;
    //    stopTransform->scaleX = 6.0f;
    //    stopTransform->scaleY = 1.0f;
    //    stopTransform->scaleZ = 1.0f;
    //    std::cout << "Using default data for stop button" << std::endl;
    //}

    m_buttonsCreated = true;
    m_currentState = GameState::MENU;
    //std::cout << "Menu buttons created successfully" << std::endl;
}

// Rest of the methods remain the same...
void GUISystem::removeMenuButtons(GameObjectManager& manager) {
    if (m_playButton) {
        manager.deleteGameObject(m_playButton);
        m_playButton = nullptr;
    }
    if (m_stopButton) {
        manager.deleteGameObject(m_stopButton);
        m_stopButton = nullptr;
    }
    m_buttonsCreated = false;
}

void GUISystem::startGame(GameObjectManager& manager) {
    std::cout << "Play button clicked - starting game" << std::endl;

    // Remove menu buttons
    removeMenuButtons(manager);

    // Load the game scene
    manager.init();
    manager.initializeSceneResources();

    // Change game state
    m_currentState = GameState::PLAYING;

    //std::cout << "Game started - loaded json" << std::endl;
}

void GUISystem::stopGame(GameObjectManager& manager) {
    std::cout << "Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }

    //std::cout << "Application quit requested" << std::endl;
}

bool GUISystem::isPointInButton(const glm::vec2& point, GameObject* button) {
    if (!button || !button->hasComponent<Transform>()) return false;

    Transform* transform = button->getComponent<Transform>();

    // Simple AABB collision check for button
    float halfWidth = transform->scaleX * 0.5f;
    float halfHeight = transform->scaleY * 0.5f;

    return (point.x >= transform->x - halfWidth &&
        point.x <= transform->x + halfWidth &&
        point.y >= transform->y - halfHeight &&
        point.y <= transform->y + halfHeight);
}