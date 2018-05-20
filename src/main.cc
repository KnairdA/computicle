#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <chrono>

#include "shader/vertex.glsl"
#include "shader/fragment.glsl"
#include "shader/compute.glsl"

const std::size_t particle_count = 100000;

GLint getUniform(GLuint program, const std::string& name) {
	const GLint uniform = glGetUniformLocation(program, name.c_str());
	if ( uniform == -1 ) {
		std::cerr << "Could not bind uniform " << name << std::endl;
	}
	return uniform;
}

GLint compileShader(const std::string& source, GLenum type) {
	GLint shader = glCreateShader(type);

	if ( !shader ) {
		std::cerr << "Cannot create a shader of type " << type << std::endl;
		exit(-1);
	}

	const char* source_data = source.c_str();
	const int source_length = source.size();

	glShaderSource(shader, 1, &source_data, &source_length);
	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if ( !compiled ) {
		std::cerr << "Cannot compile shader" << std::endl;
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		for( auto c : errorLog ) {
			std::cerr << c;
		}
		std::cerr << std::endl;
	}

	return shader;
}

GLint setupShader() {
	GLint shader = glCreateProgram();
	glAttachShader(shader, compileShader(VERTEX_SHADER_CODE, GL_VERTEX_SHADER));
	glAttachShader(shader, compileShader(FRAGMENT_SHADER_CODE, GL_FRAGMENT_SHADER));
	glLinkProgram(shader);
	return shader;
}

GLint setupComputeShader() {
	GLint shader = glCreateProgram();
	glAttachShader(shader, compileShader(COMPUTE_SHADER_CODE, GL_COMPUTE_SHADER));
	glLinkProgram(shader);
	return shader;
}

int window_width  = 800;
int window_height = 600;
float world_width, world_height;
glm::mat4 MVP;

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
	glViewport(0, 0, width, height);
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

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	updateMVP();

	GLint ShaderID  = setupShader();
	GLuint MatrixID = glGetUniformLocation(ShaderID, "MVP");

	GLint ComputeShaderID = setupComputeShader();
	GLuint WorldID = glGetUniformLocation(ComputeShaderID, "world");

	auto vertex_buffer_data = makeInitialParticles(particle_count);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	glEnableVertexAttribArray(0);

	GLuint VertexBufferID;
	glGenBuffers(1, &VertexBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VertexBufferID);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		vertex_buffer_data.size() * sizeof(GLfloat),
		vertex_buffer_data.data(),
		GL_STATIC_DRAW
	);

	auto lastFrame = std::chrono::high_resolution_clock::now();

	do {
		// update particles at most 50 times per second
		if ( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastFrame).count() >= 20 ) {
			glUseProgram(ComputeShaderID);

			glUniform2f(WorldID, world_width, world_height);
			glDispatchCompute(particle_count, 1, 1);

			glUseProgram(0);

			lastFrame = std::chrono::high_resolution_clock::now();
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(ShaderID);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glDrawArrays(GL_POINTS, 0, 3*particle_count);

		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &VertexBufferID);
	glDisableVertexAttribArray(0);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(ShaderID);
	glDeleteProgram(ComputeShaderID);

	glfwTerminate();

	return 0;
}
