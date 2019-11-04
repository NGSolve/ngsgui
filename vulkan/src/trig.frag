#version 450

layout (location = 0) smooth in vec3 inColor;
layout (location = 1) smooth in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main(){
        // float l = 0.3 + dot(inNormal, vec3(0,0,1));
	outFragColor = vec4( inColor, 1.0 );
}
