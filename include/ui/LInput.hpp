#pragma once

#include <map>
#include <cstdint>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace LInput {
	bool GetKey(uint32_t key);
	bool GetKeyDown(uint32_t key);
	bool GetKeyUp(uint32_t key);

	bool GetMouseButton(uint32_t button);
	bool GetMouseButtonDown(uint32_t button);
	bool GetMouseButtonUp(uint32_t button);

	glm::vec2 GetMousePosition();
	glm::vec2 GetMouseDelta();
	int32_t GetMouseScrollDelta();

	// Tick the internal state. For internal use only.
	void UpdateInputState();

	void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void GLFWMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
	void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void GLFWMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
}
