#pragma once

namespace GameEngine {

uint32_t numMipLevels(const wgpu::Extent3D &size);

void generateMipmap(const wgpu::Device &device, const wgpu::Texture &texture, wgpu::TextureViewDimension textureBindingViewDimension = wgpu::TextureViewDimension::Undefined);

}
