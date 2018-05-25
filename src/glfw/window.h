#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
private:
	bool _good   = false;
	int  _width  = 800;
	int  _height = 600;

	GLFWwindow* const _handle;

public:
	Window(const std::string& title):
		_handle(glfwCreateWindow(_width, _height, title.c_str(), NULL, NULL)) {
		if ( _handle != nullptr ) {
			glfwMakeContextCurrent(_handle);
			if ( glewInit() == GLEW_OK ) {
				_good = true;
			}
			glfwMakeContextCurrent(nullptr);
		}
	}

	bool isGood() const {
		return _good;
	}

	int getWidth() const {
		return _width;
	}

	int getHeight() const {
		return _height;
	}

	template <class F>
	void init(F f) {
		glfwMakeContextCurrent(_handle);
		f();
		glfwMakeContextCurrent(nullptr);
	}

	template <class F>
	void render(F loop) {
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

};
