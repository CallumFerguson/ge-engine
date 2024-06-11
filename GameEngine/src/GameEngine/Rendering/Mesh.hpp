#pragma once

#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"

namespace GameEngine {

class Mesh : public Asset {
public:
    Mesh();

    explicit Mesh(const std::string &inputFilePath);

    const wgpu::Buffer &indexBuffer();

    const wgpu::Buffer &positionBuffer();

    const wgpu::Buffer &normalBuffer();

    uint32_t indexCount();

private:
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_positionBuffer;
    wgpu::Buffer m_normalBuffer;

    uint32_t m_indexCount = 0;
};

}
