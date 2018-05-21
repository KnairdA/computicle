#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>
#include <chrono>
#include <memory>

#include "graphic_shader.h"
#include "compute_shader.h"
#include "texture_buffer.h"
#include "particle_vertex_buffer.h"

#include "shader/vertex.glsl"
#include "shader/fragment.glsl"
#include "shader/compute.glsl"

const std::size_t particle_count = 100000;

int window_width  = 800;
int window_height = 600;
float world_width, world_height;
glm::mat4 MVP;

std::unique_ptr<TextureBuffer>        textureBuffer;
std::unique_ptr<ParticleVertexBuffer> particleBuffer;

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

	GraphicShader sceneShader(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);
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

	sceneShader.setUniform("MVP", MVP);
	displayShader.setUniform("screen_texture", 0);

	textureBuffer = std::make_unique<TextureBuffer>(window_width, window_height);
	particleBuffer = std::make_unique<ParticleVertexBuffer>(
		makeInitialParticles(particle_count));

	ComputeShader computeShader(COMPUTE_SHADER_CODE);
	computeShader.workOn(particleBuffer->getBuffer());

	const std::vector<GLfloat> quad_buffer_data{
		-1.f,  1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 0.f,
		 1.f, -1.f, 1.f, 0.f,

		-1.f,  1.f, 0.f, 1.f,
		 1.f, -1.f, 1.f, 0.f,
		 1.f,  1.f, 1.f, 1.f
	};

	GLuint QuadVertexArrayID;
	GLuint QuadVertexBufferID;

	glGenVertexArrays(1, &QuadVertexArrayID);
	glGenBuffers(1, &QuadVertexBufferID);

	glBindVertexArray(QuadVertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVertexBufferID);
	glBufferData(
		GL_ARRAY_BUFFER,
		quad_buffer_data.size() * sizeof(GLfloat),
		quad_buffer_data.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));

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

			particleBuffer->draw();
		}

		{
			auto guard = displayShader.use();

			glBindVertexArray(QuadVertexArrayID);
			glBindTexture(GL_TEXTURE_2D, textureBuffer->getTexture());

			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );

	glfwTerminate();

	return 0;
}
