#pragma once

#include <string>
#include <tiny_gltf.h>

namespace GameEngineTools {

bool writeGLTFTextureImageFile(const tinygltf::Image &image, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID);

void writeFakeTexture(const uint8_t *data, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID);

}
