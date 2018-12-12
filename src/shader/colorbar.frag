#version 150
out vec4 color;
in float value;

{include utils.inc}

void main() {
    float v = mix(colormap.min, colormap.max, value);
    color = vec4(MapColor(v), 1.0);
}
