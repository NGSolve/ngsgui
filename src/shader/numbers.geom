#version 150 

{include utilsnew.inc}
#line 4
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
    vec3 p = vec3(0,0,0);
    ELEMENT_TYPE el = getElement(mesh, inData[0].element);

    for (int i=0; i<ELEMENT_N_VERTICES; i++) {
        p += el.pos[i];
    }

    p *= 1.0/ELEMENT_N_VERTICES;
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
