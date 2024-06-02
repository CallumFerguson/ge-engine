cmake_minimum_required(VERSION 3.24)
project(Tools)

set(CMAKE_CXX_STANDARD 20)

add_executable(TestTool PackageGLTF/packageGLTF.cpp)
target_link_libraries(TestTool PRIVATE GameEngine)

target_include_directories(
        TestTool PUBLIC
        ../GameEngine/src
)
