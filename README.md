# Game Engine

## Overview

This project is designed to be compatible with various operating systems, compilers, and build systems, but only the
following options listed under [Build Instructions](#build-instructions) are officially supported.

## Build Instructions

- **Windows**
    - Terminal
        - [Native](#building-windows-native-in-terminal)
        - [WASM](#building-wasm-on-windows-in-terminal)
    - CLion
        - [Native](#building-windows-native-in-clion)
        - [WASM](#building-wasm-on-windows-in-clion)

### Building windows native in terminal

```
git clone https://github.com/CallumFerguson/GameEngine
cd GameEngine
cmake -B build -G Ninja
cmake --build build
cd build/dist
.\GameEngine.exe
```

### Building WASM on windows in terminal

```
git clone https://github.com/CallumFerguson/GameEngine
cd GameEngine
emcmake cmake -B build-wasm -G Ninja -DUSE_EMSCRIPTEN=ON
cmake --build build-wasm
cd build-wasm/dist
```

Then host local server to view the build. For example:

```
python -m http.server
```

### Building Windows native in CLion

Project should be able to build Windows native using CLion without any additional setup

- press build or run

### Building WASM on windows in CLion

Python must be installed to run the web server after building

- create a .env file and add emscripten_cmake_path=C:\Program Files\JetBrains\CLion [version]
  \bin\cmake\win\x64\bin\cmake.exe
- Go to "File" > "Settings", then "Build, Execution, Deployment" > "Toolchains"
- Duplicate "MinGW (default)" and name it "MinGW-Emscripten"
- Set CMake to the included cmake-emscripten.bat
- go to "Build, Execution, Deployment" > "CMake"
- Duplicate "Debug" and name it "Debug-Emscripten"
- add CMake option -DUSE_EMSCRIPTEN=ON
- Select GameEngine-Emscripten | Debug-Emscripten configuration
- Go to "Run" > "Edit Configurations..."
- Select "CMake Application" > "GameEngine-Emscripten"
- Set Executable to the included start-http-server.bat
- Set Program arguments to $CMakeCurrentBuildDir$/dist
- Under "Before launch", add "Launch Web Browser", and put http://127.0.0.1:9229 for the URL
- Press build or run