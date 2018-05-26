#include <random>
#include <memory>
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glfw/guard.h"
#include "glfw/window.h"

#include "buffer/frame/texture_framebuffer.h"

#include "buffer/vertex/particle_vertex_buffer.h"
#include "buffer/vertex/texture_display_vertex_buffer.h"

#include "shader/wrap/graphic_shader.h"
#include "shader/wrap/compute_shader.h"

#include "shader/code/vertex.glsl"
#include "shader/code/fragment.glsl"
#include "shader/code/compute.glsl"

#include "shader/code/display_vertex.glsl"
#include "shader/code/display_fragment.glsl"

#include "timer.h"

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
		std::cerr << "Failed to open GLFW window." << std::endl;
		return -1;
	}

	int window_width  = window.getWidth();
	int window_height = window.getHeight();

	float world_width  = 20.0;
	float world_height = getWorldHeight(window_width, window_height, world_width);

	glm::mat4 MVP = getMVP(world_width,  world_height);

	std::vector<std::unique_ptr<TextureFramebuffer>> texture_framebuffers;

	std::unique_ptr<ParticleVertexBuffer> particle_buffer;
	std::unique_ptr<TextureDisplayVertexBuffer> display_buffer;

	std::unique_ptr<GraphicShader> scene_shader;
	std::unique_ptr<ComputeShader> compute_shader;
	std::unique_ptr<GraphicShader> display_shader;

	window.init([&]() {
		for ( unsigned int i = 0; i < texture_count; ++i ) {
			texture_framebuffers.emplace_back(
				new TextureFramebuffer(window_width, window_height));
		}

		particle_buffer = std::make_unique<ParticleVertexBuffer>(
			makeInitialParticles(particle_count, world_width, world_height));
		display_buffer  = std::make_unique<TextureDisplayVertexBuffer>();

		scene_shader = std::make_unique<GraphicShader>(
			VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);
		compute_shader = std::make_unique<ComputeShader>(
			getShaderFunction("cos(v.x*sin(v.y))",
			                  "sin(v.x-v.y)"));
		compute_shader->workOn(particle_buffer->getBuffer());
		display_shader = std::make_unique<GraphicShader>(
			DISPLAY_VERTEX_SHADER_CODE, DISPLAY_FRAGMENT_SHADER_CODE);
	});

	if ( std::any_of(texture_framebuffers.cbegin(), texture_framebuffers.cend(),
		[](auto& texture_framebuffer) -> bool {
			return !texture_framebuffer->isGood();
		}) ) {
		std::cerr << "Texture framebuffer error" << std::endl;
		return -1;
	}

	auto lastFrame  = timer::now();
	auto lastRotate = timer::now();
	bool justRotated = true;

	std::vector<GLuint> textures;
	for ( const auto& texture_buffer : texture_framebuffers ) {
		textures.emplace_back(texture_buffer->getTexture());
	}

	window.render([&]() {
		if (    window.getWidth()  != window_width
		     || window.getHeight() != window_height ) {
			window_width  = window.getWidth();
			window_height = window.getHeight();
			world_height  = getWorldHeight(window_width, window_height, world_width);

			MVP = getMVP(world_width, world_height);

			for ( auto& texture_buffer : texture_framebuffers ) {
				texture_buffer->resize(window_width, window_height);
			}
		}

		if ( timer::millisecondsSince(lastFrame) >= 1000/max_ups ) {
			auto guard = compute_shader->use();

			compute_shader->setUniform("world", world_width, world_height);
			compute_shader->dispatch(particle_count);

			lastFrame = timer::now();
		}

		if ( timer::millisecondsSince(lastRotate) >= 1000/10 ) {
			std::rotate(textures.begin(), textures.end()-1, textures.end());
			std::rotate(texture_framebuffers.begin(), texture_framebuffers.end()-1, texture_framebuffers.end());
			justRotated = true;

			lastRotate = timer::now();
		}

		{
			auto texGuard = texture_framebuffers[0]->use();
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
