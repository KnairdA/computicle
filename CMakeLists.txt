cmake_minimum_required(VERSION 3.10)
project(computicle)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -std=c++17 -W -Wall -Wextra -Winline -pedantic"
)

include_directories(
	src/
)

add_executable(
	computicle
		src/main.cc
		src/timer.cc
		src/glfw/guard.cc
		src/glfw/window.cc
		src/shader/util.cc
		src/shader/wrap/compute_shader.cc
		src/shader/wrap/graphic_shader.cc
		src/buffer/frame/texture_framebuffer.cc
		src/buffer/vertex/particle_vertex_buffer.cc
		src/buffer/vertex/texture_display_vertex_buffer.cc
)

target_link_libraries(
	computicle
		GL
		glfw
		GLEW
)
