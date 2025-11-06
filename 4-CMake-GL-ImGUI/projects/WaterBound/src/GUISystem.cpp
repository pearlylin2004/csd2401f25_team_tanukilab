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

    // Load the menu scene - this will populate button data in the manager
    manager.loadScene(runtimeScenePath("menu_scene.json"));

    // Get button data from the manager
    if (manager.hasButtonData()) {
        m_buttonData = manager.getButtonData();
        m_buttonDataLoaded = true;
        std::cout << "Successfully loaded " << m_buttonData.size() << " button(s) from GameObjectManager" << std::endl;
    }
    else {
        std::cout << "No button data found in GameObjectManager" << std::endl;
        m_buttonDataLoaded = false;
    }

    // Create the actual button game objects
    createMenuButtons(manager);
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
    if (m_buttonsCreated) {
        std::cout << "Buttons already created, returning early" << std::endl;
        return;
    }

    std::cout << "Creating buttons..." << std::endl;

    // Create Play Button
    m_playButton = manager.createGameObject("play_button");
    if (!m_playButton) {
        std::cout << "ERROR: Failed to create play button!" << std::endl;
        return;
    }

    Render* playRender = m_playButton->addComponent<Render>();
    playRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    playRender->hasTex = false;

    Transform* playTransform = m_playButton->addComponent<Transform>();

    // Use JSON data if available
    if (m_buttonDataLoaded && m_buttonData.find("play_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["play_button"];
        playTransform->x = data.position.x;
        playTransform->y = data.position.y;
        playTransform->z = data.position.z;
        playTransform->scaleX = data.scale.x;
        playTransform->scaleY = data.scale.y;
        playTransform->scaleZ = data.scale.z;
        std::cout << "Play button created with JSON data: pos("
            << data.position.x << ", " << data.position.y << ", " << data.position.z
            << ") scale(" << data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
    }
    else {
        // Fallback to default values
        playTransform->x = -6.0f;
        playTransform->y = 0.0f;
        playTransform->z = 0.1f;
        playTransform->scaleX = 6.0f;
        playTransform->scaleY = 1.0f;
        playTransform->scaleZ = 1.0f;
        std::cout << "Play button created with default data (no JSON data found)" << std::endl;
    }

    // Create Stop Button
    m_stopButton = manager.createGameObject("stop_button");
    if (!m_stopButton) {
        std::cout << "ERROR: Failed to create stop button!" << std::endl;
        return;
    }

    Render* stopRender = m_stopButton->addComponent<Render>();
    stopRender->modelRef = renderer::models[static_cast<int>(shape::square)];
    stopRender->hasTex = false;

    Transform* stopTransform = m_stopButton->addComponent<Transform>();

    // Use JSON data if available
    if (m_buttonDataLoaded && m_buttonData.find("stop_button") != m_buttonData.end()) {
        const ButtonData& data = m_buttonData["stop_button"];
        stopTransform->x = data.position.x;
        stopTransform->y = data.position.y;
        stopTransform->z = data.position.z;
        stopTransform->scaleX = data.scale.x;
        stopTransform->scaleY = data.scale.y;
        stopTransform->scaleZ = data.scale.z;
        std::cout << "Stop button created with JSON data: pos("
            << data.position.x << ", " << data.position.y << ", " << data.position.z
            << ") scale(" << data.scale.x << ", " << data.scale.y << ", " << data.scale.z << ")" << std::endl;
    }
    else {
        // Fallback to default values
        stopTransform->x = 3.0f;
        stopTransform->y = 0.0f;
        stopTransform->z = 0.1f;
        stopTransform->scaleX = 6.0f;
        stopTransform->scaleY = 1.0f;
        stopTransform->scaleZ = 1.0f;
        std::cout << "Stop button created with default data (no JSON data found)" << std::endl;
    }

    m_buttonsCreated = true;
    m_currentState = GameState::MENU;
    std::cout << "Menu buttons created successfully" << std::endl;
}

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
}

void GUISystem::stopGame(GameObjectManager& manager) {
    std::cout << "Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
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