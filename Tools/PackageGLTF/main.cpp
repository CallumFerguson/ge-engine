#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <tiny_gltf.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"
#include "meshPacker.hpp"
#include "texturePacker.hpp"

tinygltf::TinyGLTF loader;

std::string getFirstWordBeforeDot(const std::string &input) {
    size_t pos = input.find('.');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);
    outputFilePath /= inputFilePath.stem().string();
    std::filesystem::create_directories(outputFilePath);

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

    // TODO: handle name conflicts in mesh/texture/material/etc. save file names

    std::set<int> savedMeshes;
    std::set<int> savedMaterials;
    std::set<int> savedTextures;

    for (const auto &meshId: model.meshes) {
        GameEngine::AssetManager::createAsset<GameEngine::Mesh>({});
    }

    for (const auto &imageId: model.images) {
        GameEngine::AssetManager::createAsset<GameEngine::Texture>({});
    }

    for (const auto &materialsId: model.materials) {
        GameEngine::AssetManager::createAsset<GameEngine::Material>({});
    }

    GameEngine::Scene scene;
    std::vector<GameEngine::Entity> entities;

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        auto entity = scene.createEntity(node.name);

        auto &mesh = model.meshes[node.mesh];

        if (mesh.primitives.empty()) {
            std::cout << "mesh does not have any primitives" << std::endl;
            return 1;
        }

        if (mesh.primitives.size() > 1) {
            std::cout << "mesh has multiple primitives which is not supported yet" << std::endl;
            // TODO: just make one child for each primitive
            return 1;
        }

        auto &primitive = mesh.primitives[0];

        if (!savedMeshes.contains(node.mesh)) {
            savedMeshes.insert(node.mesh);
            GameEngineTools::writeGLTFMeshPrimitiveToFile(model, primitive, mesh.name, outputFilePath.string(), GameEngine::AssetManager::getAsset<GameEngine::Mesh>(node.mesh).assetUUID());
        }

        auto &renderer = entity.addComponent<GameEngine::PBRRendererComponent>(false);
        renderer.meshHandle = node.mesh;
        renderer.materialHandle = primitive.material;

        auto &material = model.materials[primitive.material];

        int occlusionTextureIndex = material.occlusionTexture.index;
        int metallicRoughnessTextureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        int albedoTextureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        int normalTextureIndex = material.normalTexture.index;

        if (occlusionTextureIndex == -1) {
            std::cout << "missing occlusion texture which is not supported yet" << std::endl;
            return 1;
        }
        if (metallicRoughnessTextureIndex == -1) {
            std::cout << "missing metallic toughness texture which is not supported yet" << std::endl;
            return 1;
        }
        if (albedoTextureIndex == -1) {
            std::cout << "missing albedo texture which is not supported yet" << std::endl;
            return 1;
        }
        if (normalTextureIndex == -1) {
            std::cout << "missing normal texture which is not supported yet" << std::endl;
            return 1;
        }

        bool metallicRoughnessOcclusionAllInOne = occlusionTextureIndex == metallicRoughnessTextureIndex;
        if (!metallicRoughnessOcclusionAllInOne) {
            std::cout << "metallicRoughness and occlusion must be the same texture (for now)" << std::endl;
            //TODO: combine these textures if they are different (with cpu or gpu, idk)
            return 1;
        }

        int occlusionRoughnessMetallicTextureIndex = occlusionTextureIndex;

        if (!savedMaterials.contains(primitive.material)) {
            savedMaterials.insert(node.mesh);

            std::vector<std::string> texturesUUIDs;
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[albedoTextureIndex].source).assetUUID());
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[normalTextureIndex].source).assetUUID());
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[occlusionRoughnessMetallicTextureIndex].source).assetUUID());

            if (!savedTextures.contains(albedoTextureIndex)) {
                savedTextures.insert(albedoTextureIndex);

                auto &texture = model.textures[albedoTextureIndex];
                auto &image = model.images[texture.source];

                auto &uuid = GameEngine::AssetManager::getAsset<GameEngine::Texture>(texture.source).assetUUID();
                GameEngineTools::writeGLTFTextureImageFile(image, getFirstWordBeforeDot(texture.name), outputFilePath, uuid);
            }

            if (!savedTextures.contains(normalTextureIndex)) {
                savedTextures.insert(normalTextureIndex);

                auto &texture = model.textures[normalTextureIndex];
                auto &image = model.images[texture.source];

                auto &uuid = GameEngine::AssetManager::getAsset<GameEngine::Texture>(texture.source).assetUUID();
                GameEngineTools::writeGLTFTextureImageFile(image, getFirstWordBeforeDot(texture.name), outputFilePath, uuid);
            }

            if (!savedTextures.contains(occlusionRoughnessMetallicTextureIndex)) {
                savedTextures.insert(occlusionRoughnessMetallicTextureIndex);

                auto &texture = model.textures[occlusionRoughnessMetallicTextureIndex];
                auto &image = model.images[texture.source];

                auto &uuid = GameEngine::AssetManager::getAsset<GameEngine::Texture>(texture.source).assetUUID();
                GameEngineTools::writeGLTFTextureImageFile(image, getFirstWordBeforeDot(texture.name), outputFilePath, uuid);
            }

            std::ofstream outputFile(outputFilePath / (material.name + ".gematerial"), std::ios::out | std::ios::binary);
            if (!outputFile) {
                std::cerr << "Error: Could not open file for writing!" << std::endl;
                return 1;
            }

            outputFile << GameEngine::AssetManager::getAsset<GameEngine::Material>(primitive.material).assetUUID();

            nlohmann::json materialJSON;
            materialJSON["shader"]["uuid"] = BASIC_COLOR_SHADER_UUID; // hard coded shader for now
            materialJSON["textureUUIDs"] = texturesUUIDs;

            outputFile << materialJSON;
        }

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

    std::ofstream outputFile(outputFilePath / (inputFilePath.stem().string() + ".geprefab"), std::ios::out);
    outputFile << entityJSON.dump();
}
