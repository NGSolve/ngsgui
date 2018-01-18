#version 150

uniform vec3 start_pos;
uniform float font_width_in_texture;
uniform float font_height_in_texture;
uniform float font_width_on_screen;
uniform float font_height_on_screen;

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in VertexData
{
  flat int character;
  flat int index;
} inData[];

out VertexData_out
{
  vec2 tex_coordinate;
} outData;

void main() {
    vec4 center = vec4(start_pos, 1.0) + vec4(font_width_on_screen*float(inData[0].index), 0,0,0);
    vec2 tex_center = vec2(font_width_in_texture*(inData[0].character-32),0);

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
