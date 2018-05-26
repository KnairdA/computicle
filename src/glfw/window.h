#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
private:
	bool _good   = false;
	int  _width  = 800;
	int  _height = 600;

	GLFWwindow* const _handle;

public:
	Window(const std::string& title);

	bool isGood() const;

	int getWidth() const;
	int getHeight() const;

	template <class F>
	void init(F f);

	template <class F>
	void render(F loop);
};

template <class F>
void Window::init(F f) {
	glfwMakeContextCurrent(_handle);
	f();
	glfwMakeContextCurrent(nullptr);
}

template <class F>
void Window::render(F loop) {
	glfwMakeContextCurrent(_handle);

	while ( glfwGetKey(_handle, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(_handle)        == 0 ) {
		glfwGetWindowSize(_handle, &_width, &_height);

		loop();

		glfwSwapBuffers(_handle);
		glfwPollEvents();
	}

	glfwMakeContextCurrent(nullptr);
}
