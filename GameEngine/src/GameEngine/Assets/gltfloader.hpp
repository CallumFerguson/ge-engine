#pragma once

#include <string>
#include <optional>
#include <memory>
#include <tiny_gltf.h>

namespace GameEngine {

struct MeshAsset {
    std::vector<int32_t> indices;
    std::vector<float> positions;

    MeshAsset() = delete;

    explicit MeshAsset(const std::string &inputFilePath);
};

bool writeGLTFMeshToFile(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::string &outputFilePath);

void readGLTFMesh(const std::string &outputFilePath);

}
