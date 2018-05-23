static const std::string DISPLAY_FRAGMENT_SHADER_CODE = R"(
#version 460

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screen_texture_0;
uniform sampler2D screen_texture_1;
uniform sampler2D screen_texture_2;
uniform sampler2D screen_texture_3;
uniform sampler2D screen_texture_4;
uniform sampler2D screen_texture_5;
uniform sampler2D screen_texture_6;
uniform sampler2D screen_texture_7;
uniform sampler2D screen_texture_8;
uniform sampler2D screen_texture_9;

void main() {
	FragColor =       texture(screen_texture_0, TexCoords)
	          + 0.9 * texture(screen_texture_1, TexCoords)
	          + 0.8 * texture(screen_texture_2, TexCoords)
	          + 0.7 * texture(screen_texture_3, TexCoords)
	          + 0.6 * texture(screen_texture_4, TexCoords)
	          + 0.5 * texture(screen_texture_5, TexCoords)
	          + 0.4 * texture(screen_texture_6, TexCoords)
	          + 0.3 * texture(screen_texture_7, TexCoords)
	          + 0.2 * texture(screen_texture_8, TexCoords)
	          + 0.1 * texture(screen_texture_9, TexCoords);
}
)";
