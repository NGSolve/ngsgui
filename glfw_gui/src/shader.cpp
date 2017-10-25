// #include <gui.hpp>
#include <string>
using std::string;
namespace shaders {
    string fragment_mesh = R"shader_string(
#version 150
uniform vec4 fColor;
in vec3 fPos;
void main()
{
  gl_FragColor = fColor; //vec4(0.0, 1.0, 0.0, 1.0);
}
)shader_string";

    string vertex_mesh = R"shader_string(
#version 150
uniform mat4 MVP;
in vec3 vPos;
out vec3 gPos;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    gPos = vPos;
}
)shader_string";

    string fragment_header = R"shader_string(
#version 150
uniform samplerBuffer coefficients;

flat in int fElement;

in vec3 posx;
in vec3 fLam;

vec3 hsv2rgb(vec3 c)
{
    // TODO: min, max as uniform
    float min = -1.0;
    float max = 1.0;
    c.x = (c.x-min)/(max-min);
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = (1.0 - c.x);
    c.x = floor(8*c.x)/7.0;
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = c.x*240.0/360.0;
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
)shader_string";

string fragment_main = R"shader_string(
void main()
{
  float x = fLam.x;
  float y = fLam.y;
  float z = fLam.z;
  gl_FragColor = vec4(hsv2rgb(vec3(Eval(x,y, z), 1.0, 1.0)), 1.0);
}
)shader_string";

string vertex_simple = R"shader_string(
#version 150
uniform mat4 MVP;

in vec3 vPos;

out vec3 posx;
out vec3 coefs;
flat out int fElement;
out vec3 fLam;
void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    fLam = vec3(0.0, 0.0, 0.0);
    posx = vPos; //0.5*vPos +0.5;
    fElement = gl_VertexID/3; //vIndex/3;
    int index = gl_VertexID - 3*fElement;
    if(index==0) fLam.x = 1.0;
    if(index==1) fLam.y = 1.0;
    if(index==2) fLam.z = 1.0;
}
)shader_string";

string geometry_copy = R"shader_string(
#version 420
 
layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;
 
in vec3 gPos[];
 
out vec3 fPos;
 
uniform mat4 MVP;
 
 void main() {
    for (int i=0; i<3; ++i) {
      gl_Position = MVP * vec4(gPos[i],1);
      fPos = gPos[i];
      EmitVertex();
    }
    EndPrimitive();
    for (int i=0; i<3; ++i) {
      gl_Position = MVP * vec4(gPos[i]+vec3(0,0,1),1);
      fPos = gPos[i];
      EmitVertex();
    }
    EndPrimitive();
}

)shader_string";
}
