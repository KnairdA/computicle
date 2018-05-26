#pragma once

#include <vector>

#include <GL/glew.h>

class ParticleVertexBuffer {
private:
	std::vector<GLfloat> _data;

	GLuint _array;
	GLuint _buffer;

public:
	ParticleVertexBuffer(std::vector<GLfloat>&& data);
	~ParticleVertexBuffer();

	GLuint getBuffer() const;

	void draw() const;
};
