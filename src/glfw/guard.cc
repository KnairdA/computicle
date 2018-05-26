#include "guard.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

GlfwGuard::GlfwGuard() {
	_good = glfwInit();
}

GlfwGuard::~GlfwGuard() {
	glfwTerminate();
}

bool GlfwGuard::isGood() const {
	return _good;
}
