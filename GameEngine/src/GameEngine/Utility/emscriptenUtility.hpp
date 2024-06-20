#pragma once

#include <string>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

void updateCursor();

void resetCanvas();

void writeTextureJSAsync(const wgpu::Device &device, const wgpu::Texture &texture, const uint8_t *data, int dataSize, bool shouldGenerateMipmap, int mipLevel, const std::string &imageType,
                         int readyStateIndex);

void writeTextureJSAsync(const wgpu::Device &device, const wgpu::Texture &texture, const std::string &url, bool shouldGenerateMipmap, int mipLevel, int readyStateIndex);

}
