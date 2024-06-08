#include "gltfloader.hpp"

#include <iostream>

// I don't know why this is needed
#ifdef APIENTRY
#undef APIENTRY
#endif

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>

#include "../Utility/Random.hpp"

namespace GameEngine {

int accessorItemByteLength(const tinygltf::Accessor &accessor) {
    int componentSizeInBytes = tinygltf::GetComponentSizeInBytes(static_cast<uint32_t>(accessor.componentType));
    if (componentSizeInBytes <= 0) {
        return -1;
    }

    int numComponents = tinygltf::GetNumComponentsInType(static_cast<uint32_t>(accessor.type));
    if (numComponents <= 0) {
        return -1;
    }

    return componentSizeInBytes * numComponents;
}

bool isTightlyPacked(const tinygltf::Accessor &accessor, const tinygltf::BufferView &bufferView) {
    int itemByteLength = accessorItemByteLength(accessor);
    if (itemByteLength <= 0 || itemByteLength != accessor.ByteStride(bufferView)) {
        return false;
    }
    return true;
}

#include <vector>

bool writeGLTFMeshToFile(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::filesystem::path &outputFilePath, const std::string &meshUUID) {
    if (mesh.primitives.empty()) {
        std::cout << "First mesh does not contain any primitives." << std::endl;
        return false;
    }

    if (mesh.primitives.size() > 1) {
        std::cout << "Mesh has multiple primitives" << std::endl;
    }

    const tinygltf::Primitive &primitive = mesh.primitives[0];
    auto it = primitive.attributes.find("POSITION");
    if (it == primitive.attributes.end()) {
        std::cout << "Primitive does not contain POSITION attribute." << std::endl;
        return false;
    }

    const tinygltf::Accessor &positionAccessor = model.accessors[it->second];
    auto &positionBufferView = model.bufferViews[positionAccessor.bufferView];
    auto &positionBuffer = model.buffers[positionBufferView.buffer];
    auto *positions = &positionBuffer.data[positionBufferView.byteOffset + positionAccessor.byteOffset];
    int32_t numPositions = positionAccessor.count;

    if (!isTightlyPacked(positionAccessor, positionBufferView)) {
        std::cout << "positions are not tightly packed which is not supported yet" << std::endl;
        return false;
    }

    if (primitive.indices < 0) {
        std::cout << "Primitive does not contain indices." << std::endl;
        return false;
    }

    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
    auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
    auto &indexBuffer = model.buffers[indexBufferView.buffer];
    auto *indices = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

    if (!isTightlyPacked(indexAccessor, indexBufferView)) {
        std::cout << "indices are not tightly packed which is not supported yet" << std::endl;
        return false;
    }

    std::vector<uint32_t> indicesVector;
    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        indicesVector.resize(indexAccessor.count);
        for (size_t i = 0; i < indexAccessor.count; i++) {
            indicesVector[i] = static_cast<uint32_t>(reinterpret_cast<const uint16_t *>(indices)[i]);
        }
        indices = reinterpret_cast<const unsigned char *>(indicesVector.data());
    } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        //
    } else {
        std::cout << "unknown indexAccessor componentType" << std::endl;
        return false;
    }
    int32_t numIndices = indexAccessor.count;

    std::ofstream outputFile(outputFilePath / (mesh.name + ".gemesh"), std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return false;
    }

    outputFile << meshUUID;

    outputFile.write(reinterpret_cast<const char *>(&numIndices), sizeof(numIndices));
    outputFile.write(reinterpret_cast<const char *>(indices), numIndices * sizeof(uint32_t));

    outputFile.write(reinterpret_cast<char *>(&numPositions), sizeof(numPositions));
    outputFile.write(reinterpret_cast<const char *>(positions), numPositions * sizeof(float) * 3);

    // Check if the write was successful
    if (!outputFile) {
        std::cerr << "Error: Failed to write data to file!" << std::endl;
        return false;
    }

    return true;
}

}
