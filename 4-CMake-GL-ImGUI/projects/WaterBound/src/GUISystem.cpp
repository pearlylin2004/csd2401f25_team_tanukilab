#include "GUISystem.h"
#include "Systems.h"
#include "input.h"
#include <iostream>

GUISystem::GUISystem() {
    std::cout << "GUISystem constructor called" << std::endl;
}

GUISystem::~GUISystem() {
    std::cout << "GUISystem destructor called" << std::endl;
}

void GUISystem::init(GameObjectManager& manager) {
    std::cout << "GUISystem::init started" << std::endl;
    createMenuButtons(manager);
    std::cout << "GUISystem::init completed successfully" << std::endl;
}

void GUISystem::update(GameObjectManager& manager) {
    // Only handle GUI interactions in menu state
    if (m_currentState == GameState::MENU) {
        // Check for button clicks
        if (InputHandler::isMouseButtonClicked(GLFW_MOUSE_BUTTON_LEFT)) {
            // Get mouse position as Vector2D
            Vector2D mousePos = InputHandler::getMousePosition();

            // Convert screen coordinates to world coordinates
            //glm::vec2 worldPos = convertScreenToWorld(mousePos.x, mousePos.y);
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
    std::cout << "GUISystem::createMenuButtons started" << std::endl;

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
    playTransform->x = -6.0f;
    playTransform->y = 0.0f;
    playTransform->z = 0.1f;
    playTransform->scaleX = 6.0f;
    playTransform->scaleY = 1.0f;
    playTransform->scaleZ = 1.0f;

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
    stopTransform->x = 3.0f;
    stopTransform->y = 0.0f;
    stopTransform->z = 0.1f;
    stopTransform->scaleX = 6.0f;
    stopTransform->scaleY = 1.0f;
    stopTransform->scaleZ = 1.0f;

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

    std::cout << "Game started - loaded json" << std::endl;
}

void GUISystem::stopGame(GameObjectManager& manager) {
    std::cout << "Stop button clicked - quitting application" << std::endl;

    // Close the GLFW window to exit the application
    GLFWwindow* window = glfwGetCurrentContext();
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }

    std::cout << "Application quit requested" << std::endl;
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

//glm::vec2 GUISystem::convertScreenToWorld(double screenX, double screenY) {
//    // Get window dimensions from GLFW
//    GLFWwindow* window = glfwGetCurrentContext();
//    int windowWidth, windowHeight;
//    glfwGetWindowSize(window, &windowWidth, &windowHeight);
//
//    if (windowWidth == 0 || windowHeight == 0) {
//        return glm::vec2(0.0f, 0.0f);
//    }
//
//    // Convert to normalized device coordinates (-1 to +1)
//    float ndcX = (2.0f * static_cast<float>(screenX)) / windowWidth - 1.0f;
//    float ndcY = 1.0f - (2.0f * static_cast<float>(screenY)) / windowHeight;
//
//    // Convert to world coordinates using camera parameters
//    float worldX = (ndcX * renderer::cam.ar * renderer::cam.zoom) + renderer::cam.campos.x;
//    float worldY = (ndcY * renderer::cam.zoom) + renderer::cam.campos.y;
//
//    return glm::vec2(worldX, worldY);
//}