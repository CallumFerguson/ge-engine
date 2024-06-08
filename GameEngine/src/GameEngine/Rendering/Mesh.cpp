#include "Mesh.hpp"

#include "../Utility/utility.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Utility/Random.hpp"

namespace GameEngine {

Mesh::Mesh() : m_assetUUID(Random::uuid()) {}

Mesh::Mesh(const std::string &inputFilePath) {
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: [Mesh] Could not open file " << inputFilePath << " for reading!" << std::endl;
        return;
    }

    char uuid[37];
    uuid[36] = '\0';
    inputFile.read(uuid, 36);
    m_assetUUID = uuid;

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

std::string &Mesh::assetUUID() {
    return m_assetUUID;
}

}
