cmake_minimum_required(VERSION 3.24)
project(GameEngineProject)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(GameEngine)

if (NOT EMSCRIPTEN)
    add_subdirectory(Tools)
endif ()
