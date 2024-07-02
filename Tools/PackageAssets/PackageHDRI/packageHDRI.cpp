#include <iostream>
#include <thread>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"
#include "computePreFilter.hpp"
#include "computeIrradiance.hpp"

namespace GameEngineTools {

void packageHDRI(const std::filesystem::path &inputFilePath, const std::filesystem::path &outputDir) {
    std::cout << "loading resources..." << std::endl;

    int equirectangularTextureHandle = GameEngine::AssetManager::createAsset<GameEngine::Texture>(inputFilePath.string(), wgpu::TextureFormat::RGBA32Float, true);
    auto &equirectangularTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(equirectangularTextureHandle);

    while (!equirectangularTexture.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        GameEngine::Texture::writeTextures();
    }

    GameEngine::FileStreamWriter preFilterStreamWriter(outputDir / (inputFilePath.stem().string() + "_preFilter.getexture"));
    auto preFilterUUID = GameEngine::Random::uuid();
    computePreFilter(equirectangularTexture, inputFilePath, preFilterStreamWriter, preFilterUUID);

    GameEngine::FileStreamWriter irradianceStreamWriter(outputDir / (inputFilePath.stem().string() + "_irradiance.getexture"));
    auto irradianceUUID = GameEngine::Random::uuid();
    computeIrradiance(equirectangularTexture, irradianceStreamWriter, irradianceUUID);

    std::ofstream outputFile(outputDir / (inputFilePath.stem().string() + ".geenvironmentmap"), std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
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

}
