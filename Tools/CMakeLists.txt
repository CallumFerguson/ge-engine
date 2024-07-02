cmake_minimum_required(VERSION 3.24)
project(Tools)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        PackageAssets

        PackageAssets/main.cpp

        PackageAssets/PackageGLTF/packageGLTF.cpp
        PackageAssets/PackageGLTF/meshPacker.cpp
        PackageAssets/PackageGLTF/texturePacker.cpp
        PackageAssets/PackageGLTF/tinygltfImpl.cpp

        PackageAssets/PackageHDRI/packageHDRI.cpp
        PackageAssets/PackageHDRI/computeIrradiance.cpp
        PackageAssets/PackageHDRI/computePreFilter.cpp
)
target_precompile_headers(PackageAssets REUSE_FROM GameEngine)
target_link_libraries(PackageAssets PRIVATE GameEngine)
target_include_directories(
        PackageAssets PUBLIC
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
