#pragma once

#include <webgpu/webgpu_cpp.h>

wgpu::Buffer createBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage);
