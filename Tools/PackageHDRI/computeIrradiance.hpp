#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

void computeIrradiance(GameEngine::Texture &equirectangularTexture, GameEngine::StreamWriter &streamWriter, const std::string &uuid);
