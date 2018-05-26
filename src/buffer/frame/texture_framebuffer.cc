#include "texture_framebuffer.h"

TextureFramebuffer::Guard::Guard(GLuint id):
	_id(id) {
	glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

TextureFramebuffer::Guard::~Guard() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TextureFramebuffer::Guard TextureFramebuffer::use() const {
	return Guard(_id);
}

TextureFramebuffer::TextureFramebuffer(std::size_t width, std::size_t height) {
	glGenFramebuffers(1, &_id);

	auto guard = use();

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
		_good = true;
	}
}

TextureFramebuffer::~TextureFramebuffer() {
	glDeleteFramebuffers(1, &_id);
}

bool TextureFramebuffer::isGood() const {
	return _good;
}

void TextureFramebuffer::resize(std::size_t width, std::size_t height) const {
	auto guard = use();

	glViewport(0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
}

GLuint TextureFramebuffer::getTexture() const {
	return _texture;
}
