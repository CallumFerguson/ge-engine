#pragma once

#include <string>
#include <optional>
#include <memory>
#include <tiny_gltf.h>

namespace Tools {

bool
writeGLTFMeshPrimitiveToFile(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &meshUUID);

}
