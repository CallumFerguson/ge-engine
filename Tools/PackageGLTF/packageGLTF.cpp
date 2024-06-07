#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <tiny_gltf.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"

tinygltf::TinyGLTF loader;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);

    tinygltf::Model model;
    std::string err;
    std::string warn;

    std::string inputFilePathExtension = inputFilePath.extension().string();
    bool result;
    if (inputFilePathExtension == ".gltf") {
        result = loader.LoadASCIIFromFile(&model, &err, &warn, inputFilePath.string());
    } else if (inputFilePathExtension == ".glb") {
        result = loader.LoadBinaryFromFile(&model, &err, &warn, inputFilePath.string());
    } else {
        std::cout << "unknown file extension: " << inputFilePathExtension << " extension must be either .gltf or .glb" << std::endl;
        return 1;
    }

    if (!warn.empty()) {
        std::cout << "loadModel warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "loadModel error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "loadModel ailed to load GLTF model." << std::endl;
        return 1;
    }
    if (model.scenes.size() != 1) {
        // TODO: probably just make an empty root element
        std::cout << "only gltf files with a single scene are supported. this gltf has " << model.scenes.size() << " scenes." << std::endl;
        return 1;
    }

    GameEngine::writeGLTFMeshToFile(model, model.meshes[0], outputFilePath.string());

    GameEngine::Scene scene;
    std::vector<GameEngine::Entity> entities;

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        auto entity = scene.createEntity(node.name);

        auto &mesh = model.meshes[node.mesh];

        if (mesh.primitives.size() > 1) {
            std::cout << "mesh has multiple primitives which is not supported yet" << std::endl;
            // TODO: just make one child for each primitive
            return 1;
        }

        entity.addComponent<GameEngine::PBRRendererComponent>(false).meshHandle = 123;

        entities.push_back(entity);
    }

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        for (const auto &childNodeId: node.children) {
            entities[childNodeId].setParent(entities[nodeId]);
        }
    }

    std::set<entt::entity> rootEntities;

    auto prefabRootEntity = scene.createEntity(inputFilePath.stem().string());

    for (auto &entity: entities) {
        rootEntities.insert(entity.getRootEntity().enttHandle());
    }

    for (auto &rootEntity: rootEntities) {
        GameEngine::Entity entity(rootEntity, &scene);
        entity.setParent(prefabRootEntity);
    }

    nlohmann::json entityJSON = GameEngine::entityToJSON(prefabRootEntity);

    std::ofstream outputFile("C:\\Users\\Calxf\\Documents\\CallumDocs\\prefab.json", std::ios::out);
    outputFile << entityJSON.dump();
}
