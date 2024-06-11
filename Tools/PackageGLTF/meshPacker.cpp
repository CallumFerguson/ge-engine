#include "meshPacker.hpp"

#include <iostream>
#include <vector>
#include "GameEngine/Utility/Random.hpp"

namespace GameEngineTools {

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

void writeAttributeToFile(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &attribute, std::ofstream &outputFile) {
    auto it = primitive.attributes.find(attribute);
    if (it == primitive.attributes.end()) {
        std::cout << "Primitive does not contain " << attribute << " attribute." << std::endl;
        return;
    }

    const tinygltf::Accessor &accessor = model.accessors[it->second];
    auto &bufferView = model.bufferViews[accessor.bufferView];
    auto &buffer = model.buffers[bufferView.buffer];
    auto *data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
    int32_t numEntries = accessor.count;

    if (!isTightlyPacked(accessor, bufferView)) {
        std::cout << "data is not tightly packed which is not supported yet" << std::endl;
        return;
    }

    int componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
    int numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    int entrySize = componentSize * numComponents;

    outputFile.write(reinterpret_cast<char *>(&numEntries), sizeof(numEntries));
    outputFile.write(reinterpret_cast<const char *>(data), numEntries * entrySize);
}

void
writeGLTFMeshPrimitiveToFile(const tinygltf::Model &model, const tinygltf::Primitive &primitive, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &meshUUID) {
    if (primitive.indices < 0) {
        std::cout << "Primitive does not contain indices." << std::endl;
        return;
    }

    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
    auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
    auto &indexBuffer = model.buffers[indexBufferView.buffer];
    auto *indices = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

    if (!isTightlyPacked(indexAccessor, indexBufferView)) {
        std::cout << "indices are not tightly packed which is not supported yet" << std::endl;
        return;
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
        return;
    }
    int32_t numIndices = indexAccessor.count;

    std::ofstream outputFile(outputFilePath / (name + ".gemesh"), std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    outputFile << meshUUID;

    outputFile.write(reinterpret_cast<const char *>(&numIndices), sizeof(numIndices));
    outputFile.write(reinterpret_cast<const char *>(indices), numIndices * sizeof(uint32_t));

    writeAttributeToFile(model, primitive, "POSITION", outputFile);
    writeAttributeToFile(model, primitive, "NORMAL", outputFile);
    writeAttributeToFile(model, primitive, "TEXCOORD_0", outputFile);
    writeAttributeToFile(model, primitive, "TANGENT", outputFile);

    // Check if the write was successful
    if (!outputFile) {
        std::cerr << "Error: Failed to write data to file!" << std::endl;
        return;
    }
}

}
