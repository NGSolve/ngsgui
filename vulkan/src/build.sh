ngscxx -g -O0 -c -std=c++17 main.cpp 
ngsld -l glfw -l vulkan -l ngcomp -l ngcore main.o -o main
glslc -I. -c trig.vert trig.frag
