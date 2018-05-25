static const std::string DISPLAY_VERTEX_SHADER_CODE = R"(
#version 330 core

layout (location = 0) in vec2 screen_vertex;
layout (location = 1) in vec2 texture_vertex;
out vec2 TexCoords;

void main() {
	gl_Position = vec4(screen_vertex, 0.0, 1.0);
	TexCoords = texture_vertex;
}
)";
