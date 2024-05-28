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
    append_compiler_flags("-fexceptions")

    append_linker_flags("--use-preload-cache")
    append_linker_flags("-sFORCE_FILESYSTEM")
    #    append_linker_flags("--preload-file ../assets")
    #    append_linker_flags("--preload-file ../shaders")
    #    append_linker_flags("-sALLOW_MEMORY_GROWTH")
    #    append_linker_flags("-sASYNCIFY=1")
    append_linker_flags("-sUSE_WEBGPU=1")
    append_linker_flags("-sUSE_GLFW=3")

else ()
    #    file(COPY assets DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")
    #    file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")

    #    append_linker_flags("-static-libstdc++")
    #    append_linker_flags("-static-libgcc")
endif ()

#TODO: separate vulkan/webgpu build and within webgpu build option for wasm or native webgpu, then use included glfw for vulkan
#link_directories(Dependencies/GLFW/lib)

if (NOT EMSCRIPTEN)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/dawn/third_party/glfw/src)
endif ()

set(
        SOURCE_FILES
        src/main.cpp
        src/assets/gltfloader.cpp
        src/utility/RollingAverage.cpp
        src/utility/utility.cpp
        src/engine/Entity.cpp
        src/engine/App.cpp
        src/engine/Scene.cpp
        src/engine/Window.cpp
        src/engine/ScriptableEntity.cpp
        src/sandbox/SandboxApp.cpp
        src/sandbox/TestScript.cpp
        src/sandbox/TestRenderer.cpp
        src/rendering/backends/webgpu/WebGPURenderer.cpp
        src/engine/Time.cpp
        src/sandbox/TrackFramerate.cpp
        src/engine/Random.cpp
        src/sandbox/ImGuiDemoWindow.cpp
        Dependencies/imgui/src/imgui.cpp
        Dependencies/imgui/src/imgui_demo.cpp
        Dependencies/imgui/src/imgui_draw.cpp
        Dependencies/imgui/src/imgui_impl_glfw.cpp
        Dependencies/imgui/src/imgui_impl_wgpu.cpp
        Dependencies/imgui/src/imgui_tables.cpp
        Dependencies/imgui/src/imgui_widgets.cpp
)

if (EMSCRIPTEN)
    list(
            APPEND SOURCE_FILES
            src/utility/emscriptenUtility.cpp
    )
else ()
    list(
            APPEND SOURCE_FILES
            #            src/webGPUDawn.cpp
    )
endif ()

add_executable(
        GameEngine
        ${SOURCE_FILES}
)

target_include_directories(
        GameEngine PRIVATE
        Dependencies/glm/include
        #        Dependencies/GLFW/include
        Dependencies/tinygltf/include
        Dependencies/imgui/include
        Dependencies/entt/include
)

function(copy_file_relative NEW_TARGET_NAME PATH TARGET)
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${PATH})
    set(DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/dist/${PATH})

    add_custom_command(
            OUTPUT ${DESTINATION}
            COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE} ${DESTINATION}
            DEPENDS ${SOURCE}
            COMMENT "Copying ${NEW_TARGET_NAME}..."
    )

    add_custom_target(${NEW_TARGET_NAME} ALL DEPENDS ${DESTINATION})
    add_dependencies(${TARGET} ${NEW_TARGET_NAME})
endfunction()

if (EMSCRIPTEN)
    target_include_directories(
            GameEngine PRIVATE
            Dependencies/wasm_webgpu/include
    )

    copy_file_relative(index_html index.html GameEngine)
    copy_file_relative(favicon_ico favicon.ico GameEngine)

    if (NOT EMSDK_PATH)
        message(FATAL_ERROR "Please specify the path to emsdk using the EMSDK_PATH variable.")
    endif ()

    # preload shaders
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.data ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.js
            COMMAND python ${EMSDK_PATH}/upstream/emscripten/tools/file_packager.py
            ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.data --preload shaders --js-output=${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.js
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            COMMENT "Packaging shaders..."
    )
    add_custom_target(package_shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.data ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders.js)
    add_dependencies(GameEngine package_shaders)

    # preload assets
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.data ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.js
            COMMAND python ${EMSDK_PATH}/upstream/emscripten/tools/file_packager.py
            ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.data --preload assets --js-output=${CMAKE_CURRENT_BINARY_DIR}/dist/assets.js
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets
            COMMENT "Packaging assets..."
    )
    add_custom_target(package_assets ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.data ${CMAKE_CURRENT_BINARY_DIR}/dist/assets.js)
    add_dependencies(GameEngine package_assets)
else ()
    set(DAWN_FETCH_DEPENDENCIES ON)
    add_subdirectory("dawn" EXCLUDE_FROM_ALL)
    target_link_libraries(GameEngine PRIVATE webgpu_cpp webgpu_dawn webgpu_glfw)
    target_include_directories(
            GameEngine PRIVATE
            dawn/third_party/glfw/include
    )

    # copy shaders
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            COMMENT "Packaging shaders..."
    )
    add_custom_target(package_shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/shaders)
    add_dependencies(GameEngine package_shaders)

    # copy assets
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/dist/assets
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets
            COMMENT "Packaging assets..."
    )
    add_custom_target(package_assets ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/assets)
    add_dependencies(GameEngine package_assets)
endif ()

#target_link_libraries(GameEngine PRIVATE glfw3)
