/* Start Header ************************************************************************/
/*!
\file       input.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100% of the file
\par        cheeqing.tan@digipen.edu
\date       September, 15th, 2025
\brief      A static utility class that manages all input handling for a GLFW window.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include <GLFW/glfw3.h>
#include <cstring>
#include <filesystem>
#include "mathlib.h"
#include "config.h"

/*
Keyboard key tokens
https://www.glfw.org/docs/3.3/group__keys.html

Mouse button
https://www.glfw.org/docs/3.3/group__buttons.html#ga181a6e875251fd8671654eff00f9112e

Useful macros
GLFW_MOUSE_BUTTON_LEFT
GLFW_KEY_W
GLFW_KEY_SPACE
*/

class InputHandler {
public:
	// initialize input handler with window and set up callbacks
	static void init(GLFWwindow* window, const AppConfig& cfg);
	// update input states (call once per frame)  (DO NOT call glfwPollEvents again in game loop)
	static void update();

	/* check for keyboard input */
	// check if key was just pressed (triggered once)
	static bool isKeyTriggered(int key);
	// check if key was just released
	static bool isKeyReleased(int key);
	// check if key is currently being held down
	static bool isKeyHeld(int key);
	// check if both keys are triggered (default is left shift)
	static bool isComboKeyTriggered(int key1, int key2 = GLFW_KEY_LEFT_SHIFT);
	// check if both keys are being held down (default is left shift)
	static bool isComboKeyHeld(int key1, int key2 = GLFW_KEY_LEFT_SHIFT);

	/* check for mouse button */
	// check if left mouse button was clicked
	static bool isMouseLeftClicked();
	// check if left mouse button is currently being held down
	static bool isMouseLeftHeld();
	// check if left mouse button was just released
	static bool isMouseLeftReleased();
	// check if right mouse button was just clicked
	static bool isMouseLeftDoubleClicked();
	// ONLY use if you need other buttons
	static bool isMouseButtonClicked(int button);
	// check if mouse button was double clicked (within 0.25s)
	static bool isMouseButtonDoubleClicked(int button);

	// check if mouse is being dragged (left button held and moving)
	static bool isMouseDragging();
	// get change in mouse position since drag started
	static Vector2D getMouseDragDelta();
	// get mouse position when drag started
	static Vector2D getMouseDragStartPos();

	static Vector2D getMouseDeltaWorld();

	/* Cursor Position */
	// get current cursor position
	static Vector2D getMousePosition();
	// get current cursor x position
	static float getMouseX();
	// get current cursor y position
	static float getMouseY();
	// get change in mouse position since last frame
	static Vector2D getMouseDelta();
	// check if cursor is inside window
	static bool isCursorInsideWindow();
	// check if cursor is inside ImGui viewport
	static bool isCursorInsideImGuiViewport();

	// get cursor world postion
	static Vector2D getWorldMousePosition();
	// get world cursor x position
	static float getWorldMouseX();
	// get world cursor y position
	static float getWorldMouseY();

	// get scroll offset since last frame
	static float getMouseScroll();

	/* callbacks(ignore these and DON'T touch them, GLFW will call them automatically) */
	// key callback function
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	// mouse button callback function
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	// cursor position callback function
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	// cursor enter/leave callback function
	static void CursorEnterCallback(GLFWwindow* window, int entered);
	// scroll callback function
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static void dropCallback(GLFWwindow* window, int count, const char** paths);

private:
	static GLFWwindow* m_window;
	static AppConfig m_cfg;

	static bool m_keys[1024];
	static bool m_keysPrevious[1024];

	static bool m_mouseButtons[8];
	static bool m_mouseButtonsPrevious[8];
	static double m_lastClickTime[8];
	static Vector2D m_mousePosition;
	static Vector2D m_mousePositionPrevious;
	static Vector2D m_mouseDragStartPos;
	static bool m_isDragging;
	static bool m_isCursorInside;

	static float m_scrollOffset;
	static double m_doubleClickThreshold;

};