cmake_minimum_required(VERSION 3.24)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist)

if (DEFINED USE_EMSCRIPTEN AND USE_EMSCRIPTEN)
    file(COPY index.html DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist")
endif ()

link_directories(Dependencies/GLFW/lib)

add_executable(GameEngine src/main.cpp)

target_include_directories(
        GameEngine PRIVATE
        Dependencies/glm/include
        Dependencies/GLFW/include
)

target_link_libraries(GameEngine PRIVATE glfw3)
