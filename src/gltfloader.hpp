#pragma once

#include <string>
#include <tiny_gltf.h>

struct Model {
    tinygltf::Model model;
    size_t numPositions;
    void *positions;
    size_t numIndices;
    void *indices;
    int indicesComponentType;
};

std::optional<Model> loadModel(const std::string &filename);
