#!/bin/sh
x86_64-w64-mingw32-g++ -std=c++11 -static -static-libgcc -static-libstdc++ -pthread -Iinclude/ src/*.cpp test/c2.cpp -o test/c2.exe -lws2_32 -lmswsock -ladvapi32
x86_64-w64-mingw32-g++ -std=c++11 -static -static-libgcc -static-libstdc++ -pthread -Iinclude/ src/*.cpp test/s2.cpp -o test/s2.exe -lws2_32 -lmswsock -ladvapi32

