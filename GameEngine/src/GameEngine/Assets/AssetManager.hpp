#pragma once

#include <string>
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Backends/WebGPU/WebGPUShader.hpp"

namespace GameEngine {

class AssetManager {
public:
    static int getOrLoadMeshFromUUID(const std::string &assetUUID);

    static int getOrLoadMeshFromPath(const std::string &assetPath);

    static Mesh &getMesh(int assetHandle);

    static int loadShader(const std::string &assetPath);

    static WebGPUShader &getShader(int assetHandle);
};

}
