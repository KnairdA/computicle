#pragma once

#include <cstdint>

#include <GL/glew.h>

class TextureFramebuffer {
private:
	GLuint _id;
	GLuint _texture;

	bool _good = false;

public:
	struct Guard {
		const GLuint _id;

		Guard(GLuint id);
		~Guard();
	};

	Guard use() const;

	TextureFramebuffer(std::size_t width, std::size_t height);
	~TextureFramebuffer();

	bool isGood() const;

	void resize(std::size_t width, std::size_t height) const;

	GLuint getTexture() const;
};
