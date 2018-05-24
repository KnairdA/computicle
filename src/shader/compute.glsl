static const std::string COMPUTE_SHADER_CODE = R"(
#version 430

layout (local_size_x = 1) in;
layout (std430, binding=1) buffer bufferA{ float data[]; };

uniform vec2 world;

// vector field definition

vec2 f(vec2 v) {
	return vec2(
		cos(v.x*sin(v.y)),
		sin(v.x-v.y)
	);
}

// ODE solver

vec2 explicitEuler(float h, vec2 v) {
	return v + h * f(v);
}

vec2 classicalRungeKutta(float h, vec2 v) {
	const vec2 k1 = f(v);
	const vec2 k2 = f(v + h/2. * k1);
	const vec2 k3 = f(v + h/2. * k2);
	const vec2 k4 = f(v + h    * k3);

	return v + h * (1./6.*k1 + 1./3.*k2 + 1./3.*k3 + 1./6.*k4);
}

// pseudo random numbers for particle placement

float rand(vec2 v){
	return fract(sin(dot(v, vec2(12.9898,78.233))) * 43758.5453);
}

float mapUnitToWorldX(float s) {
	return -(world.x/2.) + s * world.x;
}

float mapUnitToWorldY(float s) {
	return -(world.y/2.) + s * world.y;
}

bool insideWorld(vec2 v) {
	return v.x > -world.x/2.
	    && v.x <  world.x/2.
	    && v.y > -world.y/2.
	    && v.y <  world.y/2.;
}

void main() {
	const uint i = 3*gl_GlobalInvocationID.x;
	const vec2 v = vec2(data[i+0], data[i+1]);
	const vec2 w = classicalRungeKutta(0.01, v);

	if ( data[i+2] < 5. && insideWorld(v) ) {
		data[i+0]  = w.x;
		data[i+1]  = w.y;
		data[i+2] += 0.01;
	} else {
		data[i+0] = mapUnitToWorldX(rand(v));
		data[i+1] = mapUnitToWorldY(rand(w));
		data[i+2] = rand(v+w) * 5.;
	}
}
)";
