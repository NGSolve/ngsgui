#version 150 

{include utils.inc}

uniform mat4 P;
uniform mat4 MV;

uniform samplerBuffer vertices;
uniform Mesh mesh;

uniform float font_width_in_texture;
uniform float font_height_in_texture;
uniform float font_width_on_screen;
uniform float font_height_on_screen;

uniform vec4 clipping_plane;

layout(points) in;
layout(triangle_strip, max_vertices=40) out;

in VertexData
{
  flat int element;
} inData[];

out VertexData_out
{
vec2 tex_coordinate;
} outData;


vec3 getPosition() {
    vec3 p;
    if(mesh.dim==0) {
        p = texelFetch(vertices, inData[0].element).xyz;
    }
    if(mesh.dim==1) {
        Element1d el = getElement1d(mesh, inData[0].element);
        p = 0.7*el.pos[0] +0.3*el.pos[1];
    }
    if(mesh.dim==2) {
        Element2d el = getElement2d(mesh, inData[0].element);
        p = el.pos[0] +el.pos[1] + el.pos[2];
        p *= 1.0/3;
    }
    if(mesh.dim==3) {
        Element3d el = getElement3d(mesh, inData[0].element);
        p = el.pos[0] +el.pos[1] + el.pos[2] + el.pos[3];
        p *= 1.0/4;
    }

    return p;
}

void main() {
    // Convert number to base 10 array of digits
    int digits[10];
    int ndigits = 0;

    int n = inData[0].element;
    while(n>0)
    {
        digits[ndigits] = n%10;
        ndigits++;
        n /= 10;
    }

    if(inData[0].element == 0) {
        ndigits = 1;
        digits[0] = 0;
    }

    vec3 p = getPosition();
    if( dot(clipping_plane, vec4(p,1))<0 ) return;
    vec4 pos = P*MV*vec4(p,1);
    pos = pos/pos.w;

    for (int i=0; i<ndigits; i++) {
        // align with top left corner
        vec4 center = pos + vec4(font_width_on_screen*i, 0,0,0);

        // center at element
        if(mesh.dim>0) {
            center += vec4(-font_width_on_screen*ndigits*0.5, 0.5*font_height_on_screen, 0,0);
        }
        vec2 tex_center = vec2(font_width_in_texture*(digits[ndigits-1-i]+48-32),0);

        gl_Position =                center + vec4(0*font_width_on_screen,  0*font_height_on_screen, 0, 0);
        outData.tex_coordinate = tex_center + vec2(0*font_width_in_texture, 0*font_height_in_texture);
        EmitVertex();

        gl_Position =                center + vec4(0*font_width_on_screen, -1*font_height_on_screen, 0, 0);
        outData.tex_coordinate = tex_center + vec2(0*font_width_in_texture, 1*font_height_in_texture);
        EmitVertex();

        gl_Position =                center + vec4(1*font_width_on_screen,  0*font_height_on_screen, 0, 0);
        outData.tex_coordinate = tex_center + vec2(1*font_width_in_texture, 0*font_height_in_texture);
        EmitVertex();

        gl_Position =                center + vec4(1*font_width_on_screen, -1*font_height_on_screen, 0, 0);
        outData.tex_coordinate = tex_center + vec2(1*font_width_in_texture, 1*font_height_in_texture);
        EmitVertex();

        EndPrimitive();
    }
}
