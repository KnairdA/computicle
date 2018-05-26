#pragma once

#include <GLFW/glfw3.h>

class KeyWatcher {
private:
	GLFWwindow* const _handle;

	int _key;
	int _last_state;

public:
	KeyWatcher(GLFWwindow* handle, int key);

	bool wasClicked();

};
