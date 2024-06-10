#pragma once

#include <string>
#include <optional>
#include <memory>
#include <tiny_gltf.h>

namespace Tools {

bool writeGLTFMeshToFile(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::filesystem::path &outputFilePath, const std::string &meshUUID);

}
