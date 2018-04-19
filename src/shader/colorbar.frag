#version 150
out vec4 color;
in float value;

{include utils.inc}

void main() {
    float val = floor(8*value)/7.0;
    color = vec4(MapColor(1.0-val), 1);
}
