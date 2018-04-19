#version 150
uniform mat4 MVP;
uniform float x0;
uniform float y0;
uniform float dx;
uniform float dy;
out float value;
void main() {
    vec4 pos = vec4(x0, y0, 0, 1);
    value = 0.0;
    if(gl_VertexID == 1){
        value = 1.0;
        pos.x += dx;
    }
    if(gl_VertexID == 3) pos.y += dy;
    if(gl_VertexID == 2) {
        value = 1.0;
        pos.x += dx;
        pos.y += dy;
    }

    gl_Position = pos;
}
