#include "compute_shader.h"

#include "shader/util.h"

ComputeShader::Guard::Guard(GLuint id):
	_id(id) {
	glUseProgram(_id);
}

ComputeShader::Guard::~Guard() {
	glUseProgram(0);
}

ComputeShader::Guard ComputeShader::use() const {
	return Guard(_id);
}

ComputeShader::ComputeShader(const std::string& src):
	_id(glCreateProgram()) {
	GLint shader = util::compileShader(src, GL_COMPUTE_SHADER);

	if ( shader != -1 ) {
		glAttachShader(_id, shader);
		glLinkProgram(_id);
		_good = true;
	}
}

ComputeShader::~ComputeShader() {
	glDeleteProgram(_id);
}

bool ComputeShader::isGood() const {
	return _good;
}

GLuint ComputeShader::setUniform(const std::string& name, float x, float y) const {
	GLuint id = util::getUniform(_id, name);
	glUniform2f(id, x, y);
	return id;
}

void ComputeShader::workOn(GLuint buffer) const {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer);
}

void ComputeShader::dispatch(std::size_t dimX) const {
	glDispatchCompute(dimX, 1, 1);
}
