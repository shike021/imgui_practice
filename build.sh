#!/bin/bash
echo "=== begin build files $* ..."

#g++ main.cpp src/imgui.cpp src/* --std=c++17 -I ./include -I ./include/imGui  -I ./include/imGui/backends -I../../code/glfw/include -L ../../code/glfw/build/src -lglfw3 -framework OpenGL -framework Cocoa -framework CoreFoundation -framework IOKit
g++ -c main.cpp src/*  --std=c++17 -I ./include -I ./include/imGui  -I ./include/imGui/backends -I../../code/glfw/include      
g++ *.o -L ../../code/glfw/build/src -lglfw3 -framework OpenGL -framework Cocoa -framework CoreFoundation -framework IOKit
rm *.o
echo "=== build files finished ..."

