#include "key_watcher.h"

KeyWatcher::KeyWatcher(GLFWwindow* handle, int key):
	_handle(handle),
	_key(key),
	_last_state(glfwGetKey(_handle, _key))
{ }

bool KeyWatcher::wasClicked() {
	switch ( glfwGetKey(_handle, _key) ) {
		case GLFW_RELEASE:
			_last_state = GLFW_RELEASE;
			return false;
		case GLFW_PRESS:
			if ( _last_state == GLFW_RELEASE ) {
				_last_state = GLFW_PRESS;
				return true;
			} else {
				return false;
			}
		default:
			return false;
	}
}
