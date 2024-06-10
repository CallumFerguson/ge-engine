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

    uint32_t indexCount();

private:
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_positionBuffer;

    uint32_t m_indexCount = 0;
};

}
