#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>
#include <memory>
#include <algorithm>

#include "particle_vertex_buffer.h"
#include "texture_display_buffer.h"

#include "graphic_shader.h"
#include "compute_shader.h"
#include "texture_buffer.h"

#include "shader/vertex.glsl"
#include "shader/fragment.glsl"
#include "shader/compute.glsl"

#include "shader/display_vertex.glsl"
#include "shader/display_fragment.glsl"

const unsigned int particle_count = 1000;
const unsigned int max_ups        = 100;
const unsigned int texture_count  = 20;

unsigned int window_width  = 800;
unsigned int window_height = 600;
float world_width, world_height;
glm::mat4 MVP;

std::vector<std::unique_ptr<TextureBuffer>> textureBuffers;
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

	MVP = projection * view;
}

void window_size_callback(GLFWwindow*, int width, int height) {
	window_width  = width;
	window_height = height;

	for ( auto& textureBuffer : textureBuffers ) {
		textureBuffer->resize(width, height);
	}

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

	for ( unsigned int i = 0; i < texture_count; ++i ) {
		textureBuffers.emplace_back(
			new TextureBuffer(window_width, window_height));
	}

	particleBuffer = std::make_unique<ParticleVertexBuffer>(
		makeInitialParticles(particle_count));

	GraphicShader sceneShader(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);

	ComputeShader computeShader(COMPUTE_SHADER_CODE);
	computeShader.workOn(particleBuffer->getBuffer());

	GraphicShader displayShader(DISPLAY_VERTEX_SHADER_CODE,
	                            DISPLAY_FRAGMENT_SHADER_CODE);
	TextureDisplayBuffer displayBuffer;

	auto lastFrame  = std::chrono::high_resolution_clock::now();
	auto lastRotate = std::chrono::high_resolution_clock::now();
	bool justRotated = true;

	std::vector<GLuint> textures;
	for ( auto& textureBuffer : textureBuffers ) {
		textures.emplace_back(textureBuffer->getTexture());
	}

	do {
		if ( util::millisecondsSince(lastFrame) >= 1000/max_ups ) {
			auto guard = computeShader.use();

			computeShader.setUniform("world", world_width, world_height);
			computeShader.dispatch(particle_count);

			lastFrame = std::chrono::high_resolution_clock::now();
		}

		if ( util::millisecondsSince(lastRotate) >= 1000/10 ) {
			std::rotate(textures.begin(), textures.end()-1, textures.end());
			std::rotate(textureBuffers.begin(), textureBuffers.end()-1, textureBuffers.end());
			justRotated = true;
			lastRotate = std::chrono::high_resolution_clock::now();
		}

		{
			auto texGuard = textureBuffers[0]->use();
			auto sdrGuard = sceneShader.use();

			sceneShader.setUniform("MVP", MVP);

			if ( justRotated ) {
				glClear(GL_COLOR_BUFFER_BIT);
				justRotated = false;
			}

			particleBuffer->draw();
		}

		{
			auto guard = displayShader.use();

			displayShader.setUniform("screen_textures",      textures);
			displayShader.setUniform("screen_textures_size", textures.size());

			glClear(GL_COLOR_BUFFER_BIT);

			displayBuffer.draw(textures);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );

	glfwTerminate();

	return 0;
}
