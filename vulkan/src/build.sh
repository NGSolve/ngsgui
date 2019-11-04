ngscxx -g -O0 -c -std=c++17 new.cpp 
ngsld -l glfw -l vulkan -l ngcomp -l ngcore new.o -o new
glslc -I. -c trig.vert trig.frag
glslc -I. -c tex.vert tex.frag
