#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

std::ostringstream computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &inputFilePath);
