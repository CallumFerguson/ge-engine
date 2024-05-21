#include "gltfloader.hpp"

#include <iostream>
#include <optional>

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

tinygltf::TinyGLTF loader;

std::optional<Model> loadModel(const std::string &filename) {
    tinygltf::Model model;
    std::string err;
    std::string warn;

    bool result = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "loadModel warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "loadModel error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "loadModel ailed to load GLTF model." << std::endl;
        return std::nullopt;
    }

    if (model.meshes.empty()) {
        std::cout << "Model does not contain any meshes." << std::endl;
        return std::nullopt;
    }

    auto &mesh = model.meshes[0];
    if (mesh.primitives.empty()) {
        std::cout << "First mesh does not contain any primitives." << std::endl;
        return std::nullopt;
    }

    const tinygltf::Primitive &primitive = mesh.primitives[0];
    auto it = primitive.attributes.find("POSITION");
    if (it == primitive.attributes.end()) {
        std::cout << "Primitive does not contain POSITION attribute." << std::endl;
        return std::nullopt;
    }

    const tinygltf::Accessor &accessor = model.accessors[it->second];
    auto &bufferView = model.bufferViews[accessor.bufferView];
    auto &buffer = model.buffers[bufferView.buffer];
    void *positions = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

    if (primitive.indices < 0) {
        std::cout << "Primitive does not contain indices." << std::endl;
        return std::nullopt;
    }

    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
    auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
    auto &indexBuffer = model.buffers[indexBufferView.buffer];
    void *indices = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

    Model returnModel;
    returnModel.model = model;
    returnModel.numPositions = accessor.count;
    returnModel.positions = positions;
    returnModel.numIndices = indexAccessor.count;
    returnModel.indices = indices;
    returnModel.indicesComponentType = indexAccessor.componentType;
    return returnModel;

    // Access the first image used as a texture
//    if (!model.textures.empty()) {
//        const tinygltf::Texture &texture = model.textures.front();
//        if (texture.source >= 0) {
//            const tinygltf::Image &image = model.images[texture.source];
//            std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
//        } else {
//            std::cout << "Texture source is invalid." << std::endl;
//        }
//    } else {
//        std::cout << "No textures found in the model." << std::endl;
//    }
}
