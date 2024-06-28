#include "Mesh.hpp"

#include "../Utility/utility.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Utility/Random.hpp"

namespace GameEngine {

Mesh::Mesh() : Asset(Random::uuid()) {}

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

    uint32_t assetVersion;
    inputFile.read(reinterpret_cast<char *>(&assetVersion), sizeof(assetVersion));
    if (assetVersion != 0) {
        std::cout << "unknown mesh asset version" << std::endl;
    }

    // indices
    inputFile.read(reinterpret_cast<char *>(&m_indexCount), sizeof(m_indexCount));
    int32_t indicesByteLength = m_indexCount * sizeof(uint32_t);
    m_indexBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), indicesByteLength, wgpu::BufferUsage::Index);
    inputFile.read(reinterpret_cast<char *>(m_indexBuffer.GetMappedRange()), indicesByteLength);
    m_indexBuffer.Unmap();

    // positions
    {
        int32_t numEntries;
        inputFile.read(reinterpret_cast<char *>(&numEntries), sizeof(numEntries));
        int32_t byteLength = numEntries * sizeof(float) * 3;
        m_positionBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), byteLength, wgpu::BufferUsage::Vertex);
        inputFile.read(reinterpret_cast<char *>(m_positionBuffer.GetMappedRange()), byteLength);
        m_positionBuffer.Unmap();
    }

    // normals
    {
        int32_t numEntries;
        inputFile.read(reinterpret_cast<char *>(&numEntries), sizeof(numEntries));
        int32_t byteLength = numEntries * sizeof(float) * 3;
        m_normalBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), byteLength, wgpu::BufferUsage::Vertex);
        inputFile.read(reinterpret_cast<char *>(m_normalBuffer.GetMappedRange()), byteLength);
        m_normalBuffer.Unmap();
    }

    // uvs
    {
        int32_t numEntries;
        inputFile.read(reinterpret_cast<char *>(&numEntries), sizeof(numEntries));
        int32_t byteLength = numEntries * sizeof(float) * 2;
        m_uvBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), byteLength, wgpu::BufferUsage::Vertex);
        inputFile.read(reinterpret_cast<char *>(m_uvBuffer.GetMappedRange()), byteLength);
        m_uvBuffer.Unmap();
    }

    // tangents
    {
        int32_t numEntries;
        inputFile.read(reinterpret_cast<char *>(&numEntries), sizeof(numEntries));
        int32_t byteLength = numEntries * sizeof(float) * 4;
        m_tangentBuffer = createMappedWebGPUBuffer(WebGPURenderer::device(), byteLength, wgpu::BufferUsage::Vertex);
        inputFile.read(reinterpret_cast<char *>(m_tangentBuffer.GetMappedRange()), byteLength);
//        auto tangents = reinterpret_cast<float *>(m_tangentBuffer.GetMappedRange());
//        for (int i = 0; i < numEntries; i++) {
//            if (tangents[i * 4 + 3] != 1) {
//                std::cout << tangents[i * 4 + 3] << std::endl;
//            }
//        }
        m_tangentBuffer.Unmap();
    }
}

const wgpu::Buffer &Mesh::indexBuffer() {
    return m_indexBuffer;
}

const wgpu::Buffer &Mesh::positionBuffer() {
    return m_positionBuffer;
}

const wgpu::Buffer &Mesh::normalBuffer() {
    return m_normalBuffer;
}

const wgpu::Buffer &Mesh::uvBuffer() {
    return m_uvBuffer;
}

const wgpu::Buffer &Mesh::tangentBuffer() {
    return m_tangentBuffer;
}

uint32_t Mesh::indexCount() {
    return m_indexCount;
}

}
