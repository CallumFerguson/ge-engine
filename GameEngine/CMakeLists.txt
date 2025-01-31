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
        src/GameEngine/Assets/AssetManager.cpp
        src/GameEngine/Assets/Asset.cpp

        src/GameEngine/Utility/RollingAverage.cpp
        src/GameEngine/Utility/utility.cpp
        src/GameEngine/Utility/Random.cpp
        src/GameEngine/Utility/stbImpl.cpp
        src/GameEngine/Utility/TimingHelper.cpp

        src/GameEngine/Utility/Stream/StreamWriter.cpp
        src/GameEngine/Utility/Stream/StreamReader.cpp
        src/GameEngine/Utility/Stream/FileStreamWriter.cpp
        src/GameEngine/Utility/Stream/FileStreamReader.cpp

        src/GameEngine/Core/App.cpp
        src/GameEngine/Core/Window.cpp
        src/GameEngine/Core/Time.cpp
        src/GameEngine/Core/Input.cpp
        src/GameEngine/Core/Exit.cpp

        src/GameEngine/Scene/Scene.cpp
        src/GameEngine/Scene/Entity.cpp
        src/GameEngine/Scene/ScriptableEntity.cpp
        src/GameEngine/Scene/Components.cpp
        src/GameEngine/Scene/renderImGuiHeirarchy.cpp
        src/GameEngine/Scene/prefab.cpp

        src/GameEngine/Rendering/Backends/WebGPU/WebGPURenderer.cpp
        src/GameEngine/Rendering/Backends/WebGPU/WebGPUShader.cpp
        src/GameEngine/Rendering/Backends/WebGPU/generateMipmapWebGPU.cpp
        src/GameEngine/Rendering/Backends/WebGPU/textureUtils.cpp
        src/GameEngine/Rendering/Mesh.cpp
        src/GameEngine/Rendering/Material.cpp
        src/GameEngine/Rendering/Texture.cpp
        src/GameEngine/Rendering/CubeMap.cpp
        src/GameEngine/Rendering/EnvironmentMap.cpp

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
    target_sources(
            GameEngine
            PUBLIC
            src/GameEngine/Utility/ThreadPool.cpp
    )
endif ()

if (EMSCRIPTEN)
    target_link_options(
            GameEngine PUBLIC
            -sEXIT_RUNTIME
            -sEXPORTED_FUNCTIONS=_main,_copyExternalImageToTextureFinishCallback
            -sEXPORTED_RUNTIME_METHODS=ccall
            --js-library ${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/Utility/emscriptenUtility.js
            --use-preload-cache
            -sFORCE_FILESYSTEM
            -sASYNCIFY
            -sUSE_WEBGPU=1
            -sUSE_GLFW=3
            -sALLOW_MEMORY_GROWTH
            -sASSERTIONS
    )
    set_target_properties(GameEngine PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/GameEngine/Utility/emscriptenUtility.js)
endif ()

target_precompile_headers(GameEngine PRIVATE src/gepch.hpp)

target_include_directories(
        GameEngine PUBLIC
        ../Dependencies/glm/include
        #        ../Dependencies/GLFW/include
        ../Dependencies/imgui/include
        ../Dependencies/entt/include
        ../Dependencies/nlohmannjson/include
        ../Dependencies/nlohmannjson/include/nlohmann # since tinygltf attempts to include json.hpp, not nlohmann/json.hpp
        ../Dependencies/stb/include
)

if (NOT EMSCRIPTEN)
    # to prevent some warnings from tint
    if (WIN32)
        set(CMAKE_OBJECT_PATH_MAX 256)
    endif ()

    # this doesn't seem to work on dawn versions after 3d35d57ea306e9365c04d92208a849bfbfb1eb4a
    # maybe check the newest dawn commit in the future to see if it will build statically
    # (it actually builds statically even without this on this current dawn commit)
    set(BUILD_SHARED_LIBS OFF)

    # since dawn is not fetched with all its submodules, this will get the required dependencies
    set(DAWN_FETCH_DEPENDENCIES ON)

    # from: https://groups.google.com/g/dawn-graphics/c/u8gq_EGfFU0
    set(TINT_BUILD_TESTS OFF CACHE BOOL INTERNAL)
    set(TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS OFF CACHE STRING INTERNAL)
    set(BUILD_SAMPLES OFF CACHE BOOL INTERNAL)

    include(FetchContent)
    FetchContent_Declare(
            dawn
            GIT_REPOSITORY https://dawn.googlesource.com/dawn.git
            GIT_TAG 3d35d57ea306e9365c04d92208a849bfbfb1eb4a
            GIT_SUBMODULES ""
            GIT_SUBMODULES_RECURSE OFF
    )
    set(FETCHCONTENT_QUIET NO)
    FetchContent_GetProperties(dawn)
    if (NOT dawn_POPULATED)
        FetchContent_Populate(dawn)
        add_subdirectory(${dawn_SOURCE_DIR} ${dawn_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif ()

    target_link_libraries(GameEngine PUBLIC webgpu_cpp webgpu_dawn webgpu_glfw)
    target_include_directories(
            GameEngine PUBLIC
            ${dawn_SOURCE_DIR}/third_party/glfw/include
    )

    target_include_directories(
            GameEngine PUBLIC
            ../Dependencies/half/include
    )
endif ()

#target_link_libraries(GameEngine PRIVATE glfw3)
