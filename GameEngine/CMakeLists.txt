cmake_minimum_required(VERSION 3.24)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)

#TODO: separate vulkan/webgpu build and within webgpu build option for wasm or native webgpu, then use included glfw for vulkan
#link_directories(Dependencies/GLFW/lib)

if (NOT EMSCRIPTEN)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/dawn/third_party/glfw/src)
endif ()

add_library(
        GameEngine
        STATIC
        src/GameEngine/Assets/gltfloader.cpp

        src/GameEngine/Utility/RollingAverage.cpp
        src/GameEngine/Utility/utility.cpp
        src/GameEngine/Utility/Random.cpp

        src/GameEngine/Core/App.cpp
        src/GameEngine/Core/Window.cpp
        src/GameEngine/Core/Time.cpp
        src/GameEngine/Core/Input.cpp
        src/GameEngine/Core/Exit.cpp

        src/GameEngine/Scene/Scene.cpp
        src/GameEngine/Scene/Entity.cpp
        src/GameEngine/Scene/ScriptableEntity.cpp

        src/GameEngine/Rendering/Backends/WebGPU/WebGPURenderer.cpp
        src/GameEngine/Rendering/Backends/WebGPU/WebGPUShader.cpp

        ../Dependencies/imgui/src/imgui.cpp
        ../Dependencies/imgui/src/imgui_demo.cpp
        ../Dependencies/imgui/src/imgui_draw.cpp
        ../Dependencies/imgui/src/imgui_impl_glfw.cpp
        ../Dependencies/imgui/src/imgui_impl_wgpu.cpp
        ../Dependencies/imgui/src/imgui_tables.cpp
        ../Dependencies/imgui/src/imgui_widgets.cpp
)

if (EMSCRIPTEN)
    target_sources(
            GameEngine
            PUBLIC
            src/GameEngine/Utility/emscriptenUtility.cpp
    )
else ()
    #    target_sources(
    #            GameEngine
    #            PUBLIC
    #
    #    )
endif ()

if (EMSCRIPTEN)
    target_link_options(GameEngine PUBLIC -sEXIT_RUNTIME)
endif ()

target_precompile_headers(GameEngine PRIVATE src/gepch.hpp)

target_include_directories(
        GameEngine PUBLIC
        ../Dependencies/glm/include
        #        ../Dependencies/GLFW/include
        ../Dependencies/tinygltf/include
        ../Dependencies/imgui/include
        ../Dependencies/entt/include
)

if (NOT EMSCRIPTEN)
    set(DAWN_FETCH_DEPENDENCIES ON)
    add_subdirectory(../dawn ${CMAKE_BINARY_DIR}/dawn EXCLUDE_FROM_ALL)
    target_link_libraries(GameEngine PUBLIC webgpu_cpp webgpu_dawn webgpu_glfw)
    target_include_directories(
            GameEngine PUBLIC
            ../dawn/third_party/glfw/include
    )
endif ()

#target_link_libraries(GameEngine PRIVATE glfw3)
