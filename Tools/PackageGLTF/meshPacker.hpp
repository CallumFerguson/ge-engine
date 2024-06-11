#pragma once

#include <string>
#include <tiny_gltf.h>

namespace GameEngineTools {

void writeGLTFMeshPrimitiveToFile(
        const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &meshUUID
);

}
