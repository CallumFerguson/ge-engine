#pragma once

#include <filesystem>
#include "GameEngine.hpp"

void computeIrradiance(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &irradianceOutputPath);
