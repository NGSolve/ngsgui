fragment_mesh = """
#version 150
uniform vec4 fColor;
void main()
{
  gl_FragColor = fColor;
}
"""

vertex_mesh = """
#version 150
uniform mat4 MV;
uniform mat4 P;
in vec3 vPos;

void main()
{
    gl_Position = P * MV * vec4(vPos, 1.0);
}
"""

fragment_header = """
#version 150
uniform samplerBuffer coefficients;
uniform float colormap_min, colormap_max;
uniform bool colormap_linear;

in VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} inData;

vec3 hsv2rgb(vec3 c)
{
    // TODO: min, max as uniform
    float min = colormap_min;
    float max = colormap_max;
    c.x = (c.x-min)/(max-min);
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = (1.0 - c.x);
    if(!colormap_linear)
      c.x = floor(8*c.x)/7.0;
    c.x = clamp(c.x, 0.0, 1.0);
    c.x = c.x*240.0/360.0;
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float zahn(float x, float y) {
  return atan(1000*x*y*y - floor(1000*x*y*y));
}
"""

fragment_main = """
void main()
{
  float x = inData.lam.x;
  float y = inData.lam.y;
  float z = inData.lam.z;
  gl_FragColor = vec4(hsv2rgb(vec3(Eval(x,y, z), 1.0, 1.0)), 1.0);
//   gl_FragColor = vec4(hsv2rgb(vec3(zahn(inData.pos.x, inData.pos.y), 1.0, 1.0)), 1.0);
//   gl_FragColor = vec4(hsv2rgb(vec3(x, 1.0, 1.0)), 1.0);
  // gl_FragColor = vec4(1, 0,0,1);
}
"""

vertex_simple = """
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
"""

geometry_copy = """
#version 420

layout(triangles) in;
layout(triangle_strip, max_vertices=6) out;

in VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} inData[];

out VertexData
{
  vec3 pos;
  flat int element;
  vec3 lam;
} outData;

uniform mat4 MV;
uniform mat4 P;

void main() {
    // vec3 normal = cross(inData[1].pos-inData[0].pos, inData[2].pos-inData[0].pos);
    // normal = normal/sqrt(dot(normal,normal));

    // fBrightness = 0.3+0.7*clamp(dot(normal,vec3(1,1,1)/sqrt(3)), 0.0, 1.0);

    outData.element = inData[0].element;

    for (int i=0; i<3; ++i) {
      gl_Position = P * MV * vec4(inData[i].pos,1);
      outData.pos = inData[i].pos;
      outData.lam = inData[i].lam;
      EmitVertex();
    }
    EndPrimitive();
}

"""
