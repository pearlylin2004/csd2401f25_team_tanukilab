/* Start Header ************************************************************************/
/*!
\file       input.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486 
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 15th, 2025
\brief      This source file implements the InputHandler class, which centralizes input
			handling for GLFW. It maintains arrays of current and previous states for
			keyboard keys and mouse buttons, enabling detection of triggered, released, 
			held, and combo inputs.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "input.h"
//#include <iostream>

GLFWwindow* InputHandler::m_window = nullptr;

bool InputHandler::m_keys[1024] = { false };
bool InputHandler::m_keysPrevious[1024] = { false };

bool InputHandler::m_mouseButtons[8] = { false };
bool InputHandler::m_mouseButtonsPrevious[8] = { false };
double InputHandler::m_lastClickTime[8] = { 0.0 };
Vector2D InputHandler::m_mousePosition = Vector2D(0.0f);
Vector2D InputHandler::m_mousePositionPrevious = Vector2D(0.0f);
Vector2D InputHandler::m_mouseDragStartPos = Vector2D(0.0f);
bool InputHandler::m_isDragging = false;
bool InputHandler::m_isCursorInside = false;

float InputHandler::m_scrollOffset = 0.0f;
double InputHandler::m_doubleClickThreshold = 0.25;

void InputHandler::init(GLFWwindow* window) {
	m_window = window;

	// set up the callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetCursorEnterCallback(window, CursorEnterCallback);
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
bool InputHandler::isKeyTriggered(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is pressed this frame & was NOT pressed last frame
	return (m_keys[key] && !m_keysPrevious[key]);
}

bool InputHandler::isKeyReleased(int key) {
	if (key < 0 || key >= 1024) return false;

	// only TRUE if the key is released this frame & was pressed last frame
	return (!m_keys[key] && m_keysPrevious[key]);
}

bool InputHandler::isKeyHeld(int key) {
	if (key < 0 || key >= 1024) return false;

	// TRUE if the key is currently down
	return m_keys[key];
}

bool InputHandler::isComboKeyTriggered(int key1, int key2) {
	if (key1 < 0 || key1 >= 1024) return false;
	if (key2 < 0 || key2 >= 1024) return false;

	// key1 is held, key2 just pressed this frame
	if (m_keys[key1] && (m_keys[key2] && !m_keysPrevious[key2])) {
		return true;
	}
	// key2 is held, key1 just pressed this frame
	if (m_keys[key2] && (m_keys[key1] && !m_keysPrevious[key1])) {
		return true;
	}

	return false;
}

bool InputHandler::isComboKeyHeld(int key1, int key2) {
	if (key1 < 0 || key1 >= 1024) return false;
	if (key2 < 0 || key2 >= 1024) return false;

	return (m_keys[key1] && m_keys[key2]);
}

/* ----- MOUSE ----- */
bool InputHandler::isMouseButtonClicked(int button) {
	if (button < 0 || button >= 8) return false;

	// only TRUE if the button is pressed this frame & was NOT pressed last frame
	return (m_mouseButtons[button] && !m_mouseButtonsPrevious[button]);
}

bool InputHandler::isMouseLeftClicked() {
	return (m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && !m_mouseButtonsPrevious[GLFW_MOUSE_BUTTON_LEFT]);
}

bool InputHandler::isMouseLeftHeld() {
	return m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT];
}

bool InputHandler::isMouseLeftReleased() {
	return (!m_mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && m_mouseButtonsPrevious[GLFW_MOUSE_BUTTON_LEFT]);
}

bool InputHandler::isMouseLeftDoubleClicked() {
	return isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT);
}

bool InputHandler::isMouseButtonDoubleClicked(int button) {
	if (button < 0 || button >= 8) return false;

	if (m_mouseButtons[button] && !m_mouseButtonsPrevious[button]) {
		double currentTime = glfwGetTime();
		if ((currentTime - m_lastClickTime[button]) < m_doubleClickThreshold) { // double click if within the threshold
			m_lastClickTime[button] = 0.0; // reset last click time
			return true;
		}
		m_lastClickTime[button] = currentTime; // update last click time
	}
	return false;
}

bool InputHandler::isMouseDragging() {
	return m_isDragging;
}

Vector2D InputHandler::getMouseDragDelta() {
	if (!m_isDragging) return Vector2D(0.0f);
	
	return Vec_Sub(&m_mousePosition, &m_mouseDragStartPos);
}

Vector2D InputHandler::getMouseDragStartPos() {
	return m_mouseDragStartPos;
}

Vector2D InputHandler::getCursorPosition() {
	return m_mousePosition;
}

float InputHandler::getMouseX() {
	return m_mousePosition.x;
}

float InputHandler::getMouseY() {
	return m_mousePosition.y;
}

Vector2D InputHandler::getMouseDelta() {
	return Vec_Sub(&m_mousePosition, &m_mousePositionPrevious);
}

bool InputHandler::isCursorInsideWindow() {
	return m_isCursorInside;
}

float InputHandler::getMouseScroll() {
	return m_scrollOffset;
}


/* ----- CALLBACKS ----- */
void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key < 0 || key >= 1024) return;

	// is key is pressed, change bool in m_key to TRUE, if released then FALSE
	if (action == GLFW_PRESS) {
		m_keys[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		m_keys[key] = false;
	}
}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button < 0 || button >= 8) return;

	// is button is pressed, change bool in m_mouseButtons to TRUE, if released then FALSE
	if (action == GLFW_PRESS) {
		m_mouseButtons[button] = true;

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			m_isDragging = true;
			m_mouseDragStartPos = m_mousePosition; // record drag start position
		}
	}
	else if (action == GLFW_RELEASE) {
		m_mouseButtons[button] = false;

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			m_isDragging = false;
		}
	}
}

void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	m_mousePosition = Vector2D(static_cast<float>(xpos), static_cast<float>(ypos));
}

void InputHandler::CursorEnterCallback(GLFWwindow* window, int entered) {
	if (entered) {
		// Cursor entered the window
		m_isCursorInside = true;
	}
	else {
		// Cursor left the window
		m_isCursorInside = false;

	}
}

void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	m_scrollOffset = static_cast<float>(yoffset);
}