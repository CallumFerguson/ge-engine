cmake_minimum_required(VERSION 3.24)
project(GameEngineProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist)

add_subdirectory(src)
add_subdirectory(sandbox)
