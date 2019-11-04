ngscxx -g -c -std=c++17 main.cpp 
ngsld -l glfw -l vulkan -l ngcomp main.o -o main
glslc -I. -c trig.vert trig.frag
