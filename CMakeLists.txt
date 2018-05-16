cmake_minimum_required(VERSION 3.10)
project(shader)

set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -std=c++17 -W -Wall -Wextra -Winline -pedantic"
)

add_executable(
	shader
		main.cc
)

target_link_libraries(
	shader
		GL
		GLEW
		glfw
)
