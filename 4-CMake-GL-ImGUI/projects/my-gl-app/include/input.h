#pragma once

#include <GLFW/glfw3.h>
//#include <Math_Lib.h>
#include <mathlib.h>

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
	static void init(GLFWwindow* window);
	static void update();

	// check for keyboard input
	static bool isKeyTriggered(int key); // check if key was just pressed (triggered once)
	static bool isKeyReleased(int key);
	static bool isKeyHeld(int key); // check if key is currently being held down

	// check for mouse button
	static bool isMouseLeftClicked(); // check if left mouse button was just clicked
	static bool isMouseLeftHeld(); // check if left mouse button is currently being held down
	static bool isMouseLeftReleased(); // check if left mouse button was just released
	static bool isMouseButtonClicked(int button); // ONLY use if you need other buttons

	// get mouse position AND whether cursor is inside window
	static Vector2D getCursorPosition(); // get current cursor position
	static float getMouseX(); // get current cursor x position
	static float getMouseY(); // get current cursor y position
	static Vector2D getMouseDelta(); // get change in mouse position since last frame
	static bool isCursorInsideWindow(); // check if cursor is inside window

	static float getMouseScroll(); // get scroll offset since last frame

	// callbacks (ignore these and DON'T touch them, GLFW will call them automatically)
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void CursorEnterCallback(GLFWwindow* window, int entered);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
	static GLFWwindow* m_window;

	static bool m_keys[1024];
	static bool m_keysPrevious[1024];

	static bool m_mouseButtons[8];
	static bool m_mouseButtonsPrevious[8];
	static Vector2D m_mousePosition;
	static Vector2D m_mousePositionPrevious;
	static bool m_isCursorInside;

	static float m_scrollOffset;
};