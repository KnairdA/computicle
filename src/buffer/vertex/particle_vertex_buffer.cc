#include "particle_vertex_buffer.h"

ParticleVertexBuffer::ParticleVertexBuffer(std::vector<GLfloat>&& data):
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

ParticleVertexBuffer::~ParticleVertexBuffer() {
	glDeleteBuffers(1, &_buffer);
	glDeleteVertexArrays(1, &_array);
}

GLuint ParticleVertexBuffer::getBuffer() const {
	return _buffer;
}

void ParticleVertexBuffer::draw() const {
	glBindVertexArray(_array);
	glDrawArrays(GL_POINTS, 0, 3*_data.size());
}
