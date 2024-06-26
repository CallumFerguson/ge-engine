#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>
#include "GameEngine.hpp"

std::ostringstream computeIrradiance(GameEngine::Texture &equirectangularTexture);
