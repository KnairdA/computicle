static const std::string VERTEX_SHADER_CODE = R"(
uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(gl_Vertex.xy, 0.0, 1.0);
	gl_FrontColor = vec4(max(1. - gl_Vertex.z/5., 0.1), 0., 0., 0.);
}
)";
