#pragma once

#include <vector>

class ParticleVertexBuffer {
private:
	std::vector<GLfloat> _data;

	GLuint _array;
	GLuint _buffer;

public:
	ParticleVertexBuffer(std::vector<GLfloat>&& data):
		_data{ std::move(data) } {
		glGenVertexArrays(1, &_array);
		glGenBuffers(1, &_buffer);

		glBindVertexArray(_array);
		glBindBuffer(GL_ARRAY_BUFFER, _buffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			_data.size() * sizeof(GLfloat),
			_data.data(),
			GL_STATIC_DRAW
		);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	~ParticleVertexBuffer() {
		glDeleteBuffers(1, &_buffer);
		glDeleteVertexArrays(1, &_buffer);
	}

	void draw() {
		glBindVertexArray(_array);
		glDrawArrays(GL_POINTS, 0, 3*_data.size());
	}

	GLuint getBuffer() const {
		return _buffer;
	}
};
