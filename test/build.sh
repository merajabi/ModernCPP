#!/bin/sh
x86_64-w64-mingw32-g++ -std=c++11 -static -static-libgcc -static-libstdc++ client.cpp -o client.exe -lws2_32 -lmswsock -ladvapi32
x86_64-w64-mingw32-g++ -std=c++11 -static -static-libgcc -static-libstdc++ server.cpp -o server.exe -lws2_32 -lmswsock -ladvapi32


# I'm on x64 Windows 8 and I want to release an binary without dll's.

# I was learning DirectX (9), using Code::Blocks (13) and the 32-bit MinGW-build (4.8) with -lpthread.
# I build an standard Code::Blocks DirectX 9 project. Edited to 32-bits for both x86 and x64 and run. Ok. Then I said for my friend run it on his 64-bit computer, but he got 2 library errors: "libgcc_s_dw2-1.dll" and "libwinpthread-1.dll". After that I tryed to compile from cmd and I got errors even when including -ld3d9 and -ld3dx9. Googled up and found -static-libgcc, -static-libstdc++ and -static. None work. So, what should I do? In command prompt, I tryed:

# C:\CPP\DirectX_test>g++ -m32 -mwindows -static -static-libgcc -static-libstdc++ -lpthread -I"C:\Program Files (x86)\Microsoft DirectX SDK\Include" -L"C:\Program Files (x86)\Microsoft DirectX SDK\lib" -ld3d9 -ld3dx9 main.cpp -o DirectX_test

# And on Code::Blocks, I have the following "Other options" in Compiler:
# -m32 -static -static-libgcc -static-libstdc++ -lwinpthread -Wl,--subsystem,windows -mwindows
# And the compiling by Code::Blocks is

# g++.exe -DWIN32 -DNDEBUG -D_WINDOWS -D_MBCS -Wall -O2 -std=c++11 -m32 -static -static-libgcc -static-libstdc++ -lwinpthread -Wl,--subsystem,windows -mwindows -I"C:\Program Files (x86)\Microsoft DirectX SDK\include" -c C:\CPP\DirectX_test\main.cpp -o obj\Release\main.o

# then

# g++.exe -L"C:\Program Files (x86)\Microsoft DirectX SDK\lib" -L"C:\Program Files (x86)\Microsoft DirectX SDK\lib\x86" -o bin\Release\DirectX_test.exe obj\Release\main.o  -s  -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -ld3d9 -ld3dx9 -mwindows


