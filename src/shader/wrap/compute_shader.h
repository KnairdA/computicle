#pragma once

#include <string>

#include <GL/glew.h>

class ComputeShader {
private:
	const GLuint _id;

	bool _good;

public:
	struct Guard {
		const GLuint _id;

		Guard(GLuint id);
		~Guard();
	};

	Guard use() const;

	ComputeShader(const std::string& src);
	~ComputeShader();

	bool isGood() const;

	GLuint setUniform(const std::string& name, float x, float y) const;

	void workOn(GLuint buffer) const;
	void dispatch(std::size_t dimX) const;
};
