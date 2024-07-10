# GE Engine

![Example Screenshot](images/example-screenshot.png)

## Live Demo

Here is a [GitHub Page](https://callumferguson.github.io/ge-engine/github-pages/example/) with a live demo

## Overview

GE Engine (GE stands for Game Engine) is a simple game engine primarily designed to make it easy to experiment with
WebGPU, and eventually Vulkan.

It can build for the web using emscripten, and natively using [Dawn](https://dawn.googlesource.com/dawn)

GE Engine does not have an editor. Instead, there is a separate engine library, and engine tools executables. For
example, the PackageAssets tool should be run before every build, and it will package any assets that have not already
been packaged.

This repository includes a .run folder with CLion run configurations that will handle all of this automatically. If you
do not use CLion, you will have to set up a build script that uses the PackageAssets tools, or manually use
PackageAssets whenever you change or add any assets.

## Build Instructions

### CLion

- Download GE Engine

```
git clone https://github.com/CallumFerguson/ge-engine
```

- Open ge-engine in CLion

#### Native Build

- Select Visual Studio for the CMake Toolchain
- Select the Sandbox run configuration
- Press build or run

The Dawn repository will automatically be downloaded and built by CMake. This may take a while. If you use the run
configurations that come in this repo, assets will be automatically packaged before each run.

#### Emscripten Build

Building natively is a prerequisite for building with emscripten because the PackageAssets tool must run natively to
package the assets for the emscripten build. If you use the run configurations that come in this repo, assets will be
automatically packaged before each run using a Release build of PackageAssets.

- Create a System Toolchain called Emscripten
- Select emcc and em++ for the c and cpp compilers
    - path\to\emsdk\upstream\emscripten\emcc.bat
    - path\to\emsdk\upstream\emscripten\em++.bat
- create cmake profiles for debug/release
- select Emscripten toolchain
- set CMAKE_TOOLCHAIN_FILE CMake option
    - -DCMAKE_TOOLCHAIN_FILE=path\to\emsdk\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake
- debug build compiles faster and has more debugging information and logging, but creates builds with very bad
  performance
- release builds compile slower, and create much smaller builds, but hides exceptions, etc.
- make a third CMake profile called Debug-Faster-Emscripten that uses debug and the additional CMake option
  -DDEBUG_FASTER=ON. This will use optimization level O2 which is a good balance between compile speed, exception
  handling, and performance
- Edit run configuration Sandbox-Emscripten, and under "Before Launch", edit External tool "External
  Tools/PreloadAssetsEmscripten"
- make sure it uses
    - program: ge-engine\Sandbox\platform\emscripten\scripts\preloadAssetsEmscripten.bat
    - arguments: $CMakeCurrentBuildDir$ path\to\emsdk
    - working directory: ge-engine\Sandbox
- Press build or run
