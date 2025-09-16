#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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
	static bool isKeyPressed(int key);
	static bool isKeyReleased(int key);
	static bool isKeyDown(int key);

	// check for mouse input
	static bool isMouseButtonPressed(int button);
	static glm::vec2 getMousePosition();
	static float getMouseX();
	static float getMouseY();
	static glm::vec2 getMouseDelta();
	static float getMouseScroll();

	// callbacks
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
	static GLFWwindow* m_window;

	static bool m_keys[1024];
	static bool m_keysPrevious[1024];

	static bool m_mouseButtons[8];
	static bool m_mouseButtonsPrevious[8];
	static glm::vec2 m_mousePosition;
	static glm::vec2 m_mousePositionPrevious;

	static float m_scrollOffset;
};