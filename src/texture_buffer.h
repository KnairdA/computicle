#pragma once

class TextureBuffer {
private:
	GLuint _id;
	GLuint _texture;

public:
	struct Guard {
		const GLuint _id;

		Guard(GLuint id): _id(id) {
			glBindFramebuffer(GL_FRAMEBUFFER, _id);
		}
		~Guard() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};

	Guard use() {
		return Guard(_id);
	}

	TextureBuffer(std::size_t width, std::size_t height) {
		glGenFramebuffers(1, &_id);

		auto guard = use();

		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			std::cerr << "Texture framebuffer error" << std::endl;
		}
	}

	~TextureBuffer() {
		glDeleteFramebuffers(1, &_id);
	}

	void resize(std::size_t width, std::size_t height) {
		auto guard = use();

		glViewport(0, 0, width, height);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
	}

	GLuint getTexture() const {
		return _texture;
	}
};
