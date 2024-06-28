#include <iostream>
#include <thread>
#include <filesystem>
#include <nlohmann/json.hpp>
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
    outputFilePath /= inputFilePath.stem();
    std::filesystem::create_directories(outputFilePath);

    int equirectangularTextureHandle = GameEngine::AssetManager::createAsset<GameEngine::Texture>(inputFilePath.string(), wgpu::TextureFormat::RGBA32Float, true);
    auto &equirectangularTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(equirectangularTextureHandle);

    while (!equirectangularTexture.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        GameEngine::Texture::writeTextures();
    }

    GameEngine::FileStreamWriter preFilterStreamWriter(outputFilePath / (inputFilePath.stem().string() + "_preFilter.getexture"));
    auto preFilterUUID = GameEngine::Random::uuid();
    computePreFilter(equirectangularTexture, inputFilePath, preFilterStreamWriter, preFilterUUID);

    GameEngine::FileStreamWriter irradianceStreamWriter(outputFilePath / (inputFilePath.stem().string() + "_irradiance.getexture"));
    auto irradianceUUID = GameEngine::Random::uuid();
    computeIrradiance(equirectangularTexture, irradianceStreamWriter, irradianceUUID);

    std::ofstream outputFile(outputFilePath / (inputFilePath.stem().string() + ".geenvironmentmap"), std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return 1;
    }

    outputFile << GameEngine::Random::uuid();

    uint32_t assetVersion = 0;
    outputFile.write(reinterpret_cast<const char *>(&assetVersion), sizeof(assetVersion));

    nlohmann::json environmentMapJSON;
    environmentMapJSON["preFilterTextureUUID"] = preFilterUUID;
    environmentMapJSON["irradianceTextureUUID"] = irradianceUUID;
    outputFile << environmentMapJSON;

    std::cout << "done" << std::endl;
}
