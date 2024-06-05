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
    using json = nlohmann::json;

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

//    json prefab;
//    prefab["components"] = {
//            {
//                    "type", "NameComponent",
//                    "properties", {
//                                          "name",     "Entity",
//                                  }
//            },
//            {
//                    "type", "TransformComponent",
//                    "properties", {
//                                          "position", {0, 0, 0},
//                                          "rotation", {0, 0, 0, 1},
//                                          "scale", {1, 1, 1},
//                                  }
//            }
//    };
//    prefab["children"] = json::array();

    GameEngine::Scene scene;
    std::vector<GameEngine::Entity> entities;

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        auto entity = scene.createEntity(node.name);
        entities.push_back(entity);
    }

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        for (const auto &childNodeId: node.children) {
            entities[childNodeId].setParent(entities[nodeId]);
            std::cout << "set parent" << std::endl;

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

//    std::cout << prefabRootEntity.getComponent<GameEngine::NameComponent>().name << std::endl;

//    std::ofstream outputFile(outputFilePath, std::ios::out);
//    outputFile << prefab.dump(2);
}
