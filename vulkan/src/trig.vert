#version 450

layout(binding = 0) uniform ViewProj
{
  mat4 view;
} viewProj;


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) smooth out vec3 outColor;
layout (location = 1) smooth out vec3 outNormal;

void main(){
	outColor = vec3(0.0, 1.0, 0.0);
        float lam = 0.99;
	gl_Position = viewProj.view * vec4( inPos, 1.0 );
}
