#include "Mesh.hpp"

#include "../Utility/utility.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"

namespace GameEngine {

Mesh::Mesh(const std::string &inputFilePath) {
    std::ifstream inputFile(inputFilePath, std::ios::in | std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Could not open file for reading!" << std::endl;
        return;
    }

    inputFile.read(reinterpret_cast<char *>(&m_indexCount), sizeof(m_indexCount));
    int32_t indicesByteLength = m_indexCount * sizeof(uint32_t);
    m_indexBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), indicesByteLength, wgpu::BufferUsage::Index);
    inputFile.read(reinterpret_cast<char *>(m_indexBuffer.GetMappedRange()), indicesByteLength);
    m_indexBuffer.Unmap();

    int32_t numPositions;
    inputFile.read(reinterpret_cast<char *>(&numPositions), sizeof(numPositions));
    int32_t positionsByteLength = numPositions * sizeof(float) * 3;
    m_positionBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), positionsByteLength, wgpu::BufferUsage::Vertex);
    inputFile.read(reinterpret_cast<char *>(m_positionBuffer.GetMappedRange()), positionsByteLength);
    m_positionBuffer.Unmap();
}

const wgpu::Buffer &Mesh::indexBuffer() {
    return m_indexBuffer;
}

const wgpu::Buffer &Mesh::positionBuffer() {
    return m_positionBuffer;
}

uint32_t Mesh::indexCount() {
    return m_indexCount;
}

}
