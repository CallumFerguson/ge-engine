#pragma once

#include <filesystem>
#include "GameEngine.hpp"

void computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &preFilterOutputPath);
