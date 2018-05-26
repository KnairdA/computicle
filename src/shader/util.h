#pragma once

#include <string>

#include <GL/glew.h>

namespace util {

GLint getUniform(GLuint program, const std::string& name);

GLint compileShader(const std::string& source, GLenum type);

}
