#include "graphic_shader.h"

#include "shader/util.h"

GraphicShader::Guard::Guard(GLuint id):
	_id(id) {
	glUseProgram(_id);
}

GraphicShader::Guard::~Guard() {
	glUseProgram(0);
}

GraphicShader::Guard GraphicShader::use() const {
	return Guard(_id);
}

GraphicShader::GraphicShader(const std::string& vertex, const std::string fragment):
	_id(glCreateProgram()) {
	glAttachShader(_id, util::compileShader(vertex, GL_VERTEX_SHADER));
	glAttachShader(_id, util::compileShader(fragment, GL_FRAGMENT_SHADER));
	glLinkProgram(_id);
}

GraphicShader::~GraphicShader() {
	glDeleteProgram(_id);
}

GLuint GraphicShader::setUniform(const std::string& name, int value) const {
	GLuint id = util::getUniform(_id, name);
	glUniform1i(id, value);
	return id;
}

GLuint GraphicShader::setUniform(const std::string& name, const std::vector<GLuint>& v) const {
	GLuint id = util::getUniform(_id, name);
	glUniform1iv(id, v.size(), reinterpret_cast<const GLint*>(v.data()));
	return id;
}

GLuint GraphicShader::setUniform(const std::string& name, glm::mat4& M) const {
	GLuint id = util::getUniform(_id, name);
	glUniformMatrix4fv(id, 1, GL_FALSE, &M[0][0]);
	return id;
}
