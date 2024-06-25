#pragma once

#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

void computeIrradiance(GameEngine::Texture &equirectangularTexture, std::ofstream &outputFile);
