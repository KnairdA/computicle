#pragma once

#include "shader/util.h"

class GraphicShader {
private:
	const GLuint _id;

public:
	struct Guard {
		const GLuint _id;

		Guard(GLuint id): _id(id) {
			glUseProgram(_id);
		}
		~Guard() {
			glUseProgram(0);
		}
	};

	Guard use() const {
		return Guard(_id);
	}

	GraphicShader(const std::string& vertex, const std::string fragment):
		_id(glCreateProgram()) {
		glAttachShader(_id, util::compileShader(vertex, GL_VERTEX_SHADER));
		glAttachShader(_id, util::compileShader(fragment, GL_FRAGMENT_SHADER));
		glLinkProgram(_id);
	};
	~GraphicShader() {
		glDeleteProgram(_id);
	}

	GLuint setUniform(const std::string& name, int value) const {
		GLuint id = util::getUniform(_id, name);
		glUniform1i(id, value);
		return id;
	}

	GLuint setUniform(const std::string& name, const std::vector<GLuint>& v) const {
		GLuint id = util::getUniform(_id, name);
		glUniform1iv(id, v.size(), reinterpret_cast<const GLint*>(v.data()));
		return id;
	}

	GLuint setUniform(const std::string& name, glm::mat4& M) const {
		GLuint id = util::getUniform(_id, name);
		glUniformMatrix4fv(id, 1, GL_FALSE, &M[0][0]);
		return id;
	}
};
