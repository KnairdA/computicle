#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GlfwGuard {
private:
	bool _good = false;
public:
	GlfwGuard() {
		_good = glfwInit();
	}
	~GlfwGuard() {
		glfwTerminate();
	}

	bool isGood() const {
		return _good;
	}
};
