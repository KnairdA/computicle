#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

class GraphicShader {
private:
	const GLuint _id;

public:
	struct Guard {
		const GLuint _id;

		Guard(GLuint id);
		~Guard();
	};

	Guard use() const;

	GraphicShader(const std::string& vertex, const std::string fragment);
	~GraphicShader();

	GLuint setUniform(const std::string& name, int value) const;
	GLuint setUniform(const std::string& name, const std::vector<GLuint>& v) const;
	GLuint setUniform(const std::string& name, glm::mat4& M) const;
};
