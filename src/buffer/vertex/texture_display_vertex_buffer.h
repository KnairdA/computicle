#pragma once

#include <vector>

#include <GL/glew.h>

class TextureDisplayVertexBuffer {
private:
	const std::vector<GLfloat> _data;

	GLuint _array;
	GLuint _buffer;

public:
	TextureDisplayVertexBuffer();
	~TextureDisplayVertexBuffer();

	GLuint getBuffer() const;

	void draw(const std::vector<GLuint>& textures) const;
};
