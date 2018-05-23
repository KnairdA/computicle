#pragma once

#include <vector>

class TextureDisplayBuffer {
private:
	const std::vector<GLfloat> _data;

	GLuint _array;
	GLuint _buffer;

public:
	TextureDisplayBuffer():
		_data{
			-1.f,  1.f, 0.f, 1.f,
			-1.f, -1.f, 0.f, 0.f,
			 1.f, -1.f, 1.f, 0.f,

			-1.f,  1.f, 0.f, 1.f,
			 1.f, -1.f, 1.f, 0.f,
			 1.f,  1.f, 1.f, 1.f
		} {
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
		glVertexAttribPointer(
			0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
	}

	~TextureDisplayBuffer() {
		glDeleteBuffers(1, &_buffer);
		glDeleteVertexArrays(1, &_array);
	}

	void draw(const std::vector<GLuint>& textures) const {
		glBindVertexArray(_array);

		for ( unsigned int i = 0; i < textures.size(); ++i ) {
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	GLuint getBuffer() const {
		return _buffer;
	}
};
