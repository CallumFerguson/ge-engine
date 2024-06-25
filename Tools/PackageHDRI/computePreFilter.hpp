#pragma once

#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

void computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &inputFilePath, std::ofstream &outputFile);
