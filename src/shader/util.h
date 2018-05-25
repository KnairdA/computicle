#pragma once

#include <iostream>

namespace util {

GLint getUniform(GLuint program, const std::string& name) {
	const GLint uniform = glGetUniformLocation(program, name.c_str());
	if ( uniform == -1 ) {
		std::cerr << "Could not bind uniform " << name << std::endl;
	}
	return uniform;
}

GLint compileShader(const std::string& source, GLenum type) {
	GLint shader = glCreateShader(type);

	if ( !shader ) {
		std::cerr << "Cannot create a shader of type " << type << std::endl;
		exit(-1);
	}

	const char* source_data = source.c_str();
	const int source_length = source.size();

	glShaderSource(shader, 1, &source_data, &source_length);
	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if ( !compiled ) {
		std::cerr << "Cannot compile shader" << std::endl;
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		for( auto c : errorLog ) {
			std::cerr << c;
		}
		std::cerr << std::endl;
	}

	return shader;
}

}
