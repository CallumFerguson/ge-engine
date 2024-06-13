#pragma once

#include <string>
#include <tiny_gltf.h>

namespace GameEngineTools {

void writeGLTFTextureImageFile(const tinygltf::Image &image, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID);

}
