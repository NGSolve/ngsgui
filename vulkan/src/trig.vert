#version 450

#include <utils.inc>

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) smooth out vec3 outColor;
layout (location = 1) smooth out vec3 outPos;
layout (location = 2) smooth out vec3 outNormal;

void main(){
	outColor = vec3(0.0, 1.0, 0.0);
        float lam = 0.99;
        outPos = inPos;
	gl_Position = matrices.MVP * vec4( inPos, 1.0 );
}
