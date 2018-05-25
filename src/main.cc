#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random>
#include <memory>
#include <algorithm>

#include "glfw_guard.h"
#include "window.h"

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

const unsigned int particle_count = 2500;
const unsigned int max_ups        = 100;
const unsigned int texture_count  = 20;

float getWorldHeight(int window_width, int window_height, float world_width) {
	return world_width / window_width * window_height;
}

glm::mat4 getMVP(float world_width, float world_height) {
	const glm::mat4 projection = glm::ortho(
		-(world_width /2), world_width/2,
		-(world_height/2), world_height/2,
		0.1f, 100.0f
	);

	const glm::mat4 view = glm::lookAt(
		glm::vec3(0,0,1),
		glm::vec3(0,0,0),
		glm::vec3(0,1,0)
	);

	return projection * view;
}

std::vector<GLfloat> makeInitialParticles(std::size_t count, float world_width, float world_height) {
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

std::string getShaderFunction(const std::string& fx, const std::string& fy) {
	return COMPUTE_SHADER_CODE
	    + "vec2 f(vec2 v) {"
	    +     "return vec2(" + fx + "," + fy + ");"
	    + "}";
}

int main() {
	GlfwGuard glfw;

	if( !glfw.isGood() ) {
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	Window window("computicle");

	if ( !window.isGood() ) {
		std::cerr << "Failed to open window." << std::endl;
		return -1;
	}

	int window_width  = window.getWidth();
	int window_height = window.getHeight();

	float world_width  = 20.0;
	float world_height = getWorldHeight(window_width, window_height, world_width);

	glm::mat4 MVP = getMVP(world_width,  world_height);

	std::vector<std::unique_ptr<TextureBuffer>> texture_buffers;

	std::unique_ptr<ParticleVertexBuffer> particle_buffer;
	std::unique_ptr<TextureDisplayBuffer> display_buffer;

	std::unique_ptr<GraphicShader> scene_shader;
	std::unique_ptr<ComputeShader> compute_shader;
	std::unique_ptr<GraphicShader> display_shader;

	window.init([&]() {
		for ( unsigned int i = 0; i < texture_count; ++i ) {
			texture_buffers.emplace_back(
				new TextureBuffer(window_width, window_height));
		}

		particle_buffer = std::make_unique<ParticleVertexBuffer>(
			makeInitialParticles(particle_count, world_width, world_height));
		display_buffer  = std::make_unique<TextureDisplayBuffer>();

		scene_shader = std::make_unique<GraphicShader>(
			VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);
		compute_shader = std::make_unique<ComputeShader>(
			getShaderFunction("cos(v.x*sin(v.y))",
			                  "sin(v.x-v.y)"));
		compute_shader->workOn(particle_buffer->getBuffer());
		display_shader = std::make_unique<GraphicShader>(
			DISPLAY_VERTEX_SHADER_CODE, DISPLAY_FRAGMENT_SHADER_CODE);
	});

	auto lastFrame  = std::chrono::high_resolution_clock::now();
	auto lastRotate = std::chrono::high_resolution_clock::now();
	bool justRotated = true;

	std::vector<GLuint> textures;
	for ( const auto& texture_buffer : texture_buffers ) {
		textures.emplace_back(texture_buffer->getTexture());
	}

	window.render([&]() {
		if (    window.getWidth()  != window_width
		     || window.getHeight() != window_height ) {
			window_width  = window.getWidth();
			window_height = window.getHeight();
			world_height  = getWorldHeight(window_width, window_height, world_width);

			MVP = getMVP(world_width, world_height);

			for ( auto& texture_buffer : texture_buffers ) {
				texture_buffer->resize(window_width, window_height);
			}
		}

		if ( util::millisecondsSince(lastFrame) >= 1000/max_ups ) {
			auto guard = compute_shader->use();

			compute_shader->setUniform("world", world_width, world_height);
			compute_shader->dispatch(particle_count);

			lastFrame = std::chrono::high_resolution_clock::now();
		}

		if ( util::millisecondsSince(lastRotate) >= 1000/10 ) {
			std::rotate(textures.begin(), textures.end()-1, textures.end());
			std::rotate(texture_buffers.begin(), texture_buffers.end()-1, texture_buffers.end());
			justRotated = true;

			lastRotate = std::chrono::high_resolution_clock::now();
		}

		{
			auto texGuard = texture_buffers[0]->use();
			auto sdrGuard = scene_shader->use();

			scene_shader->setUniform("MVP", MVP);

			if ( justRotated ) {
				glClear(GL_COLOR_BUFFER_BIT);
				justRotated = false;
			}

			particle_buffer->draw();
		}

		{
			auto guard = display_shader->use();

			display_shader->setUniform("screen_textures",      textures);
			display_shader->setUniform("screen_textures_size", textures.size());

			glClear(GL_COLOR_BUFFER_BIT);

			display_buffer->draw(textures);
		}
	});

	return 0;
}
