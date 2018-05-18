#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <chrono>

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
	#define GLSL(shader) #shader
	GLint shader = glCreateProgram();

	glAttachShader(shader, compileShader(GLSL(
		uniform mat4 MVP;

		float distance(vec2 v, vec2 w) {
			vec2 u = v - w;
			return sqrt(u.x*u.x + u.y*u.y);
		}

		void main() {
			gl_Position = MVP * vec4(gl_Vertex.xy, 0.0, 1.0);
			gl_FrontColor = vec4(max(1. - gl_Vertex.z/5., 0.1), 0., 0., 0.);
		}),
		GL_VERTEX_SHADER));

	glAttachShader(shader, compileShader(GLSL(
		void main() {
			gl_FragColor = gl_Color;
		}),
		GL_FRAGMENT_SHADER));

	glLinkProgram(shader);
	return shader;
}

GLint setupComputeShader() {
	GLint shader = glCreateProgram();
	glAttachShader(shader, compileShader("#version 430\n" GLSL(
		layout (local_size_x = 1) in;
		layout (std430, binding=1) buffer bufferA{ float data[]; };
		uniform vec2 world;

		float rand(vec2 co){
			return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
		}

		bool insideWorld(vec2 v) {
			return v.x > -world.x/2.
			    && v.x <  world.x/2.
				&& v.y > -world.y/2.
				&& v.y <  world.y/2.;
		}

		void main() {
			uint idx = 3*gl_GlobalInvocationID.x;
			vec2 v = vec2(data[idx+0], data[idx+1]);

			if ( data[idx+2] < 5. && insideWorld(v) ) {
				data[idx+0] += 0.01 * cos(v.x*cos(v.y));
				data[idx+1] += 0.01 * sin(v.x-v.y);
				data[idx+2] += 0.01;
			} else {
				data[idx+0] = -(world.x/2.) + rand(vec2(data[idx+1], data[idx+0])) * world.x;
				data[idx+1] = -(world.y/2.) + rand(vec2(data[idx+0], data[idx+1])) * world.y;
				data[idx+2] = uint(rand(v) * 5.);
			}
		}),
		GL_COMPUTE_SHADER));
	glLinkProgram(shader);
	return shader;
}

int window_width  = 800;
int window_height = 600;
double mouse_x, mouse_y;
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

void cursor_pos_callback(GLFWwindow*, double x, double y) {
	mouse_x = -(world_width/2) + world_width * (x / window_width);
	mouse_y = world_height/2 - world_height * (y / window_height);
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
	if( window == NULL ){
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwMakeContextCurrent(window);

	if ( glewInit() != GLEW_OK ) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLint ShaderID = setupShader();
	GLuint MatrixID = glGetUniformLocation(ShaderID, "MVP");
	GLuint MouseID  = glGetUniformLocation(ShaderID, "mouse");

	GLint ComputeShaderID = setupComputeShader();
	GLuint WorldID = glGetUniformLocation(ComputeShaderID, "world");

	updateMVP();

	const std::size_t particle_count = 500000;

	auto vertex_buffer_data = makeInitialParticles(particle_count);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(GLfloat), &vertex_buffer_data[0], GL_STATIC_DRAW);

	auto lastFrame = std::chrono::high_resolution_clock::now();

	do {
		// update particles at most 50 times per second
		if ( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastFrame).count() >= 20 ) {
			glUseProgram(ComputeShaderID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexbuffer);
			glUniform2f(WorldID, world_width, world_height);
			glDispatchCompute(particle_count, 1, 1);

			lastFrame = std::chrono::high_resolution_clock::now();
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(ShaderID);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform2f(MouseID, mouse_x, mouse_y);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDrawArrays(GL_POINTS, 0, 3*particle_count);

		glDisableVertexAttribArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(ShaderID);
	glDeleteProgram(ComputeShaderID);

	glfwTerminate();

	return 0;
}
