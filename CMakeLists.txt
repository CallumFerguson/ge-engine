cmake_minimum_required(VERSION 3.24)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)

link_directories(Dependencies/GLFW/lib)

add_executable(GameEngine src/main.cpp)

target_include_directories(
        GameEngine PRIVATE
        Dependencies/glm/include
        Dependencies/GLFW/include
)

target_link_libraries(GameEngine PRIVATE glfw3)
