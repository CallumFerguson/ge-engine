cmake_minimum_required(VERSION 3.24)
project(Sandbox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist)

add_executable(
        Sandbox

        src/main.cpp

        src/sandboxApp.cpp

        src/scripts/TestRenderer.cpp
        src/scripts/TrackFramerate.cpp
        src/scripts/ImGuiDemoWindow.cpp
        src/scripts/CameraController.cpp
        src/scripts/Rotator.cpp
        src/scripts/PBRColorRandomizer.cpp
        src/scripts/FullscreenTexture.cpp

        src/utility/utility.cpp
)

if (EMSCRIPTEN)
    target_link_options(
            Sandbox PRIVATE
            --use-preload-cache
            -sFORCE_FILESYSTEM
            -sASYNCIFY
            -sUSE_WEBGPU=1
            -sUSE_GLFW=3
            -sALLOW_MEMORY_GROWTH
            -sNO_DISABLE_EXCEPTION_CATCHING # probably turn off for release build?
            -sASSERTIONS
            -O2
    )
    set_target_properties(Sandbox PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/GameEngine/src/GameEngine/Utility/emscriptenUtility.js)
    target_compile_options(
            Sandbox PRIVATE
            -O2
    )

    target_link_options(
            GameEngine PRIVATE
            -O2
    )
    target_compile_options(
            GameEngine PRIVATE
            -O2
    )
endif ()

target_precompile_headers(Sandbox REUSE_FROM GameEngine)

target_link_libraries(Sandbox PRIVATE GameEngine)

target_include_directories(
        Sandbox PUBLIC
        ../GameEngine/src
)

include(${CMAKE_SOURCE_DIR}/cmake/copyFileRelative.cmake)

if (EMSCRIPTEN)
    copy_file_relative(index_html SandBox/webBuildStuff index.html Sandbox)
    copy_file_relative(favicon_ico SandBox/webBuildStuff favicon.ico Sandbox)
    copy_file_relative(parse_hdr_worker_js GameEngine/src/GameEngine/Utility parseHDRWorker.js GameEngine)

    if (NOT EMSDK_PATH)
        message(FATAL_ERROR "Please specify the path to emsdk using the EMSDK_PATH variable.")
    endif ()

    # preload assets
    add_custom_command(
            OUTPUT package_assets_always_run_hack
            COMMAND python ${EMSDK_PATH}/upstream/emscripten/tools/file_packager.py ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.data --preload assets --js-output=${CMAKE_CURRENT_BINARY_DIR}/dist/assets.js --quiet
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Packaging assets..."
    )
    add_custom_target(package_assets ALL DEPENDS package_assets_always_run_hack)
    add_dependencies(Sandbox package_assets)
else ()
    # copy assets
    add_custom_command(
            OUTPUT package_assets_always_run_hack
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/dist/assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/dist/assets
            COMMENT "Packaging assets..."
    )
    add_custom_target(package_assets ALL DEPENDS package_assets_always_run_hack)
    add_dependencies(Sandbox package_assets)
endif ()
