#include "gltfloader.hpp"

#include <iostream>

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

void loadModelAndPrintVertexCount(const std::string &filename) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Load the GLTF model from file
    std::cout << "loading" << std::endl;
    bool result = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    std::cout << "finished loading" << std::endl;
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "Failed to load GLTF model." << std::endl;
        return;
    }

    // Check if the model contains any meshes
    if (model.meshes.empty()) {
        std::cout << "Model does not contain any meshes." << std::endl;
        return;
    }

    // Access the first mesh
    const tinygltf::Mesh &mesh = model.meshes[0];
    if (mesh.primitives.empty()) {
        std::cout << "First mesh does not contain any primitives." << std::endl;
        return;
    }

    // Access the first primitive of the first mesh
    const tinygltf::Primitive &primitive = mesh.primitives[0];
    auto it = primitive.attributes.find("POSITION");
    if (it == primitive.attributes.end()) {
        std::cout << "Primitive does not contain POSITION attribute." << std::endl;
        return;
    }

    // Get the number of vertices from the POSITION accessor
    const tinygltf::Accessor &accessor = model.accessors[it->second];
    std::cout << "Number of vertices in the first mesh: " << accessor.count << std::endl;

    // Access the first image used as a texture
    if (!model.textures.empty()) {
        const tinygltf::Texture &texture = model.textures.front();
        if (texture.source >= 0) {
            const tinygltf::Image &image = model.images[texture.source];
            std::cout << "Width: " << image.width << ", Height: " << image.height << std::endl;
        } else {
            std::cout << "Texture source is invalid." << std::endl;
        }
    } else {
        std::cout << "No textures found in the model." << std::endl;
    }
}
