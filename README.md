# GE Engine

![Example Screenshot](images/example-screenshot.png)

## Live Demo

Check out the [GitHub Page](https://callumferguson.github.io/ge-engine/github-pages/example/) for a live demo.

## Overview

**GE Engine (Game Engine Engine)** is a simple game engine primarily designed to make it easy to experiment with WebGPU,
and eventually Vulkan. It supports building for the web using Emscripten and natively
using [Dawn](https://dawn.googlesource.com/dawn).

GE Engine has a separate engine library and engine tools executables, but does not include an editor. For
example, the `PackageAssets` tool should be run before every build to package any new or changed assets.

This repository includes a `.run` folder with CLion run configurations to automate these tasks. If you don't use CLion,
you'll need to set up a build script that uses the `PackageAssets` tool or run `PackageAssets` manually whenever you
modify assets.

## Build Instructions

### CLion Setup

1. **Clone the Repository:**

   ```sh
   git clone https://github.com/CallumFerguson/ge-engine
   ```

2. **Open GE Engine in CLion.**

#### Native Build

1. **Configure CMake Toolchain:**
    - Select Visual Studio for the CMake Toolchain.

2. **Select Run Configuration:**
    - Choose the `Sandbox` run configuration.

3. **Build or Run:**
    - Press build or run.

CMake will automatically download and build the Dawn repository, which may take some time. Using the provided run
configurations will ensure assets are automatically packaged before each run.

#### Emscripten Build

1. **Prerequisite:**
    - Perform a native build first since the `PackageAssets` tool must run natively to package assets for the Emscripten
      build. The run configurations will automatically package assets before each run using a Release build
      of `PackageAssets`.

2. **Create Emscripten Toolchain:**
    - Create a System Toolchain called Emscripten.
    - Select `emcc` and `em++` for the C and C++ compilers:
        - `path\to\emsdk\upstream\emscripten\emcc.bat`
        - `path\to\emsdk\upstream\emscripten\em++.bat`

3. **Create CMake Profiles:**
    - Create profiles for Debug and Release builds.
    - Select the Emscripten toolchain.
    - Set the CMake Toolchain File option:
        - `-DCMAKE_TOOLCHAIN_FILE=path\to\emsdk\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake`

4. **Build Types:**
    - **Debug Build:** Compiles faster, includes more debugging information and logging but has poor performance.
    - **Release Build:** Compiles slower, creates smaller builds, and hides exceptions but offers better performance.
    - **Debug-Faster-Emscripten Build:** Create a third CMake profile using Debug and the additional CMake
      option `-DDEBUG_FASTER=ON` for a balance between compile speed, exception handling, and performance (uses
      optimization level O2).

5. **Edit Run Configuration:**
    - Edit the `Sandbox-Emscripten` run configuration.
    - Under "Before Launch," edit the External Tool `External Tools/PreloadAssetsEmscripten`:
        - Program: `ge-engine\Sandbox\platform\emscripten\scripts\preloadAssetsEmscripten.bat`
        - Arguments: `$CMakeCurrentBuildDir$ path\to\emsdk`
        - Working Directory: `ge-engine\Sandbox`

6. **Build or Run:**
    - Press build or run.
