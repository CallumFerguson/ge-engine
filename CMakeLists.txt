cmake_minimum_required(VERSION 3.24)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist)

# macros from https://github.com/juj/wasm_webgpu/blob/main/samples/CMakeLists.txt
macro(append_compiler_flags FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
endmacro()

macro(append_linker_flags FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

if (EMSCRIPTEN)
    file(COPY index.html DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")
    file(COPY favicon.ico DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")

    append_compiler_flags("-fexceptions")

    append_linker_flags("--use-preload-cache")
    append_linker_flags("--preload-file ../assets")
    append_linker_flags("--preload-file ../shaders")
    #    append_linker_flags("-sALLOW_MEMORY_GROWTH")
    append_linker_flags("-sASYNCIFY=1")
    append_linker_flags("-sUSE_WEBGPU=1")
else ()
    file(COPY assets DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")
    file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")

    append_linker_flags("-static-libstdc++")
    append_linker_flags("-static-libgcc")
endif ()

link_directories(Dependencies/GLFW/lib) # TODO: only for native? because emscripten is bundled with glfw I think?

set(
        SOURCE_FILES
        src/main.cpp
        src/gltfloader.cpp
)

if (EMSCRIPTEN)
    list(
            APPEND SOURCE_FILES
            src/webgpu.cpp
    )
endif ()

add_executable(
        GameEngine
        ${SOURCE_FILES}
)

target_include_directories(
        GameEngine PRIVATE
        Dependencies/glm/include
        Dependencies/GLFW/include
        Dependencies/tinygltf/include
)

if (EMSCRIPTEN)
    target_include_directories(
            GameEngine PRIVATE
            Dependencies/wasm_webgpu/include
    )
endif ()

target_link_libraries(GameEngine PRIVATE glfw3)
