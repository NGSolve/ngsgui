#version 450

layout(binding = 0) uniform ViewProj
{
  mat4 view;
} viewProj;


layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) smooth out vec3 outColor;

void main(){
	outColor = inColor;
        float lam = 0.99;
	gl_Position = viewProj.view * vec4( inPos.xy, 0.0, 1.0 );
}
