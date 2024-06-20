cmake_minimum_required(VERSION 3.24)
project(Tools)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        TestTool

        PackageGLTF/main.cpp

        PackageGLTF/meshPacker.cpp
        PackageGLTF/tinygltfImpl.cpp
        PackageGLTF/texturePacker.cpp
)
target_precompile_headers(TestTool REUSE_FROM GameEngine)
target_link_libraries(TestTool PRIVATE GameEngine)
target_include_directories(
        TestTool PUBLIC
        ../GameEngine/src

        ../Dependencies/tinygltf/include
)

add_executable(
        CalculateBRDF

        CalculateBRDF/main.cpp
)
target_precompile_headers(CalculateBRDF REUSE_FROM GameEngine)
target_link_libraries(CalculateBRDF PRIVATE GameEngine)
target_include_directories(
        CalculateBRDF PUBLIC
        ../GameEngine/src
)

add_executable(
        PackageHDRI

        PackageHDRI/main.cpp
)
target_precompile_headers(PackageHDRI REUSE_FROM GameEngine)
target_link_libraries(PackageHDRI PRIVATE GameEngine)
target_include_directories(
        PackageHDRI PUBLIC
        ../GameEngine/src
)
