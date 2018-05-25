#pragma once

#include "util.h"

class ComputeShader {
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

	ComputeShader(const std::string& src):
		_id(glCreateProgram()) {
		glAttachShader(_id, util::compileShader(src, GL_COMPUTE_SHADER));
		glLinkProgram(_id);
	};
	~ComputeShader() {
		glDeleteProgram(_id);
	}

	GLuint setUniform(const std::string& name, float x, float y) const {
		GLuint id = util::getUniform(_id, name);
		glUniform2f(id, x, y);
		return id;
	}

	void workOn(GLuint buffer) const {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer);
	}

	void dispatch(std::size_t dimX) const {
		glDispatchCompute(dimX, 1, 1);
	}
};
