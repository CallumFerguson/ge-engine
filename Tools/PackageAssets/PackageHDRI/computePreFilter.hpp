#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

void computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &inputFilePath, GameEngine::StreamWriter &streamWriter, const std::string &uuid);
