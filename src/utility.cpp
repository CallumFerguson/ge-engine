#include "utility.hpp"

wgpu::Buffer createBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = byteLength;
    bufferDescriptor.usage = usage;
    bufferDescriptor.mappedAtCreation = true;
    auto buffer = device.CreateBuffer(&bufferDescriptor);

    auto mappedRange = buffer.GetMappedRange();
    std::memcpy(mappedRange, data, bufferDescriptor.size);
    buffer.Unmap();

    return buffer;
}
