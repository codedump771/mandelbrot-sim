set shell := ["powershell"]

build: 
    cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    
run:
    .\build\mandelbrot.exe
    
br: build run 

build_debug: 
    cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug 
    cmake --build build 

clean: 
    rm -Recurse -Force .\build 