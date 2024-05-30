cmake_minimum_required(VERSION 3.24)
project(tools)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(testTool testTool.cpp)
target_link_libraries(testTool PRIVATE GameEngine)
