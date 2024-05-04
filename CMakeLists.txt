cmake_minimum_required(VERSION 3.24)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist)

# macro from https://github.com/juj/wasm_webgpu/blob/main/samples/CMakeLists.txt
macro(append_linker_flags FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

if (DEFINED USE_EMSCRIPTEN AND USE_EMSCRIPTEN)
    file(COPY index.html DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")
    file(COPY favicon.ico DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_WEBGPU=1")

    append_linker_flags("--js-library ${PROJECT_SOURCE_DIR}/Dependencies/wasm_webgpu/include/lib_webgpu.js")
endif ()

link_directories(Dependencies/GLFW/lib)

add_executable(
        GameEngine
        src/main.cpp
        Dependencies/wasm_webgpu/include/lib_webgpu.cpp
        Dependencies/wasm_webgpu/include/lib_webgpu_cpp20.cpp
)

target_include_directories(
        GameEngine PRIVATE
        Dependencies/glm/include
        Dependencies/GLFW/include
        Dependencies/wasm_webgpu/include
)

target_link_libraries(GameEngine PRIVATE glfw3)
