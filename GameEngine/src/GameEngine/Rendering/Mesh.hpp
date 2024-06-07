#pragma once

#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

class Mesh {
public:
    Mesh() = delete;

    explicit Mesh(const std::string &inputFilePath);

    const wgpu::Buffer &indexBuffer();

    const wgpu::Buffer &positionBuffer();

    uint32_t indexCount();

    std::string &assetUUID();

private:
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_positionBuffer;

    uint32_t m_indexCount;

    std::string m_assetUUID;
};

}
