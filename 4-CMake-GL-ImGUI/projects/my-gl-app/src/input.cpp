#include "input.h"
//#include <iostream>

GLFWwindow* InputHandler::m_window = nullptr;

bool InputHandler::m_keys[1024] = { false };
bool InputHandler::m_keysPrevious[1024] = { false };

bool InputHandler::m_mouseButtons[8] = { false };
bool InputHandler::m_mouseButtonsPrevious[8] = { false };
glm::vec2 InputHandler::m_mousePosition = glm::vec2(0.0f);
glm::vec2 InputHandler::m_mousePositionPrevious = glm::vec2(0.0f);

float InputHandler::m_scrollOffset = 0.0f;

void InputHandler::init(GLFWwindow* window) {
	m_window = window;

	// set up the callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);
}

void InputHandler::update() {
	// store previous states
	memcpy(m_keysPrevious, m_keys, sizeof(m_keys));
	memcpy(m_mouseButtonsPrevious, m_mouseButtons, sizeof(m_mouseButtons));
	m_mousePositionPrevious = m_mousePosition;

	// reset scroll each frame
	m_scrollOffset = 0.0f;

	glfwPollEvents();
}


/* ----- KEYBOARD ----- */
bool InputHandler::isKeyPressed(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is pressed this frame & was NOT pressed last frame
	return (m_keys[key] && !m_keysPrevious[key]);
}

bool InputHandler::isKeyReleased(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is released this frame & was pressed last frame
	return (!m_keys[key] && m_keysPrevious[key]);
}

bool InputHandler::isKeyDown(int key) {
	if (key < 0 || key >= 1024) return false;

	// TRUE if the key is currently down
	return m_keys[key];
}


/* ----- MOUSE ----- */
bool InputHandler::isMouseButtonPressed(int button) {
	if (button < 0 || button >= 8) return false;

	// only TRUE if the button is pressed this frame & was NOT pressed last frame
	return (m_mouseButtons[button] && !m_mouseButtonsPrevious[button]);
}

glm::vec2 InputHandler::getMousePosition() {
	return m_mousePosition;
}

float InputHandler::getMouseX() {
	return m_mousePosition.x;
}

float InputHandler::getMouseY() {
	return m_mousePosition.y;
}

glm::vec2 InputHandler::getMouseDelta() {
	return m_mousePosition - m_mousePositionPrevious;
}

float InputHandler::getMouseScroll() {
	return m_scrollOffset;
}


/* ----- CALLBACKS ----- */
void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key < 0 || key >= 1024) return;

	// is key is pressed, change bool in m_key to TRUE, if released then FALSE
	if (action == GLFW_PRESS) m_keys[key] = true;
	else if (action == GLFW_RELEASE) m_keys[key] = false;
}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button < 0 || button >= 8) return;

	// is button is pressed, change bool in m_mouseButtons to TRUE, if released then FALSE
	if (action == GLFW_PRESS) m_mouseButtons[button] = true;
	else if (action == GLFW_RELEASE) m_mouseButtons[button] = false;
}

void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	m_mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}

void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	m_scrollOffset = static_cast<float>(yoffset);
}