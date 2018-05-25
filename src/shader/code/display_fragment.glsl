static const std::string DISPLAY_FRAGMENT_SHADER_CODE = R"(
#version 460

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screen_textures[64];
uniform int screen_textures_size;

void main() {
	for ( int i = 0; i < screen_textures_size; ++i ) {
		FragColor += (1.0 - i*1.0/screen_textures_size) * texture(screen_textures[i], TexCoords);
	}
}
)";
