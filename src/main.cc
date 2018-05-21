#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <memory>

#include "graphic_shader.h"
#include "compute_shader.h"
#include "texture_buffer.h"

#include "shader/vertex.glsl"
#include "shader/fragment.glsl"
#include "shader/compute.glsl"

const std::size_t particle_count = 100000;

int window_width  = 800;
int window_height = 600;
float world_width, world_height;
glm::mat4 MVP;
std::unique_ptr<TextureBuffer> textureBuffer;

void updateMVP() {
	world_width  = 20.f;
	world_height = world_width / window_width * window_height;
	glm::mat4 projection = glm::ortho(
		-(world_width /2), world_width/2,
		-(world_height/2), world_height/2,
		0.1f, 100.0f
	);
	glm::mat4 view = glm::lookAt(
		glm::vec3(0,0,20),
		glm::vec3(0,0,0),
		glm::vec3(0,1,0)
	);
	glm::mat4 model = glm::mat4(1.0f);
	MVP = projection * view * model;
}

void window_size_callback(GLFWwindow*, int width, int height) {
	window_width  = width;
	window_height = height;

	textureBuffer->resize(width, height);

	updateMVP();
}

std::vector<GLfloat> makeInitialParticles(std::size_t count) {
	std::vector<GLfloat> buffer;
	buffer.reserve(3*count);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<GLfloat> distX(-world_width/2., world_width/2.);
	std::uniform_real_distribution<GLfloat> distY(-world_height/2., world_height/2.);
	std::uniform_real_distribution<GLfloat> distAge(0., 5.);

	for ( std::size_t i = 0; i < count; ++i ) {
		buffer.emplace_back(distX(gen));
		buffer.emplace_back(distY(gen));
		buffer.emplace_back(distAge(gen));
	}

	return buffer;
}

int main() {
	if( !glfwInit() ) {
		std::cerr <<  "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* const window = glfwCreateWindow(window_width, window_height, "computicle", NULL, NULL);

	if( window == nullptr ){
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwMakeContextCurrent(window);

	if ( glewInit() != GLEW_OK ) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	updateMVP();

	textureBuffer = std::make_unique<TextureBuffer>(window_width, window_height);

	{
		auto guard = textureBuffer->use();

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	}

	GraphicShader sceneShader(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);
	sceneShader.setUniform("MVP", MVP);

	ComputeShader computeShader(COMPUTE_SHADER_CODE);

	auto vertex_buffer_data = makeInitialParticles(particle_count);

	GLuint VertexArrayID;
	GLuint VertexBufferID;

	{
		auto guard = textureBuffer->use();

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &VertexBufferID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VertexBufferID);

		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			vertex_buffer_data.size() * sizeof(GLfloat),
			vertex_buffer_data.data(),
			GL_STATIC_DRAW
		);
	}

	GLuint QuadVertexArrayID;
	glGenVertexArrays(1, &QuadVertexArrayID);
	glBindVertexArray(QuadVertexArrayID);
	GLuint QuadVertexBufferID;
	glGenBuffers(1, &QuadVertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVertexBufferID);
	const std::vector<GLfloat> quad_buffer_data{
		-1.f,  1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 0.f,
		 1.f, -1.f, 1.f, 0.f,

		-1.f,  1.f, 0.f, 1.f,
		 1.f, -1.f, 1.f, 0.f,
		 1.f,  1.f, 1.f, 1.f
	};
	glBufferData(
		GL_ARRAY_BUFFER,
		quad_buffer_data.size() * sizeof(GLfloat),
		quad_buffer_data.data(),
		GL_STATIC_DRAW
	);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GraphicShader displayShader(R"(
		#version 330 core
		layout (location = 0) in vec2 screen_vertex;
		layout (location = 1) in vec2 texture_vertex;
		out vec2 TexCoords;

		void main() {
			gl_Position = vec4(screen_vertex, 0.0, 1.0);
			TexCoords = texture_vertex;
		}
		)", R"(
		#version 330 core
		out vec4 FragColor;
		in vec2 TexCoords;
		uniform sampler2D screen_texture;

		void main() {
			FragColor = texture(screen_texture, TexCoords);
		}
		)");
	displayShader.setUniform("screen_texture", 0);

	auto lastFrame = std::chrono::high_resolution_clock::now();

	do {
		// update particles at most 50 times per second
		if ( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastFrame).count() >= 20 ) {
			auto guard = computeShader.use();

			computeShader.setUniform("world", world_width, world_height);
			computeShader.dispatch(particle_count);

			lastFrame = std::chrono::high_resolution_clock::now();
		}

		{
			auto texGuard = textureBuffer->use();
			auto sdrGuard = sceneShader.use();

			glClear(GL_COLOR_BUFFER_BIT);

			sceneShader.setUniform("MVP", MVP);

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glDrawArrays(GL_POINTS, 0, 3*particle_count);
		}

		{
			auto guard = displayShader.use();

			glBindTexture(GL_TEXTURE_2D, textureBuffer->getTexture());
			glBindBuffer(GL_ARRAY_BUFFER, QuadVertexBufferID);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));

			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &VertexBufferID);
	glDisableVertexAttribArray(0);
	glDeleteVertexArrays(1, &VertexArrayID);

	glfwTerminate();

	return 0;
}
