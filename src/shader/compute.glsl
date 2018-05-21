static const std::string COMPUTE_SHADER_CODE = R"(
#version 430

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
		data[idx+0] += 0.01 * cos(v.x*sin(v.y));
		data[idx+1] += 0.01 * sin(v.x-v.y);
		data[idx+2] += 0.01;
	} else {
		data[idx+0] = -(world.x/2.) + rand(vec2(data[idx+1], data[idx+0])) * world.x;
		data[idx+1] = -(world.y/2.) + rand(vec2(data[idx+0], data[idx+1])) * world.y;
		data[idx+2] = uint(rand(v) * 5.);
	}
}
)";
