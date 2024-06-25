#include <iostream>
#include <thread>
#include <filesystem>
#include "GameEngine.hpp"
#include "computePreFilter.hpp"
#include "computeIrradiance.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    GameEngine::TimingHelper time("Total");

    GameEngine::WebGPURenderer::init(nullptr, {wgpu::FeatureName::Float32Filterable});

    std::cout << "loading resources..." << std::endl;

    GameEngine::AssetManager::registerAssetUUIDs("../../Sandbox/assets");

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);
    std::filesystem::create_directories(outputFilePath);

    int equirectangularTextureHandle = GameEngine::AssetManager::createAsset<GameEngine::Texture>(inputFilePath.string(), wgpu::TextureFormat::RGBA32Float, true);
    auto &equirectangularTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(equirectangularTextureHandle);

    while (!equirectangularTexture.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        GameEngine::Texture::writeTextures();
    }

    std::filesystem::path outputPath = outputFilePath / (inputFilePath.stem().string() + ".geenvironment");
    std::ofstream outputFile(outputPath, std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return 1;
    }

    outputFile << GameEngine::Random::uuid();

    computePreFilter(equirectangularTexture, inputFilePath, outputFile);

    computeIrradiance(equirectangularTexture, outputFile);

    std::cout << "done" << std::endl;
}
