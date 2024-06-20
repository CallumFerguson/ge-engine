#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <half.hpp>
#include "GameEngine.hpp"

using half_float::half;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    GameEngine::WebGPURenderer::init(nullptr);

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);
    std::filesystem::create_directories(outputFilePath);
    outputFilePath /= inputFilePath.stem().string() + ".png";

    auto device = GameEngine::WebGPURenderer::device();

    int equirectangularTextureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>(inputFilePath.string());
    auto &equirectangularTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(equirectangularTextureHandle);
}
