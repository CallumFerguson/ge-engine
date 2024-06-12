#pragma once

#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

void updateCursor();

void resetCanvas();

void writeTextureJS(const wgpu::Device &device, const wgpu::Texture &texture, const std::vector<uint8_t> &data);

}
