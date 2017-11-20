
#version 150
uniform mat4 MV;
uniform mat4 P;

in vec3 vPos;
in int vIndex;

out VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} outData;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
    outData.lam = vec3(0.0, 0.0, 0.0);
    outData.pos = vPos; //0.5*vPos +0.5;
    outData.element = gl_VertexID/3; //vIndex/3;
    if(vIndex==0) outData.lam.x = 1.0;
    if(vIndex==1) outData.lam.y = 1.0;
    if(vIndex==2) outData.lam.z = 1.0;
}

