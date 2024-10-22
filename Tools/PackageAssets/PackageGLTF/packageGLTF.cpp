#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <tiny_gltf.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"
#include "meshPacker.hpp"
#include "texturePacker.hpp"

namespace GameEngineTools {

static tinygltf::TinyGLTF loader;

static std::string getFirstWordBeforeDot(const std::string &input) {
    size_t pos = input.find('.');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input;
}

void packageGLTF(const std::filesystem::path &inputFilePath, const std::filesystem::path &outputDir) {
    std::string inputFileName = inputFilePath.stem().string();

    tinygltf::Model model;
    std::string err;
    std::string warn;

    std::cout << "loading gltf file " << inputFileName << "..." << std::endl;

    GameEngine::TimingHelper time("loaded gltf file");

    std::string inputFilePathExtension = inputFilePath.extension().string();
    bool result;
    if (inputFilePathExtension == ".gltf") {
        result = loader.LoadASCIIFromFile(&model, &err, &warn, inputFilePath.string());
    } else if (inputFilePathExtension == ".glb") {
        result = loader.LoadBinaryFromFile(&model, &err, &warn, inputFilePath.string());
    } else {
        std::cout << "unknown file extension: " << inputFilePathExtension << " extension must be either .gltf or .glb" << std::endl;
        return;
    }

    time.stop();

    if (!warn.empty()) {
        std::cout << "loadModel warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "loadModel error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "loadModel ailed to load GLTF model." << std::endl;
        return;
    }
    if (model.scenes.size() != 1) {
        // TODO: probably just make an empty root element
        std::cout << "only gltf files with a single scene are supported. this gltf has " << model.scenes.size() << " scenes." << std::endl;
        return;
    }

    // TODO: handle name conflicts in mesh/texture/material/etc. save file names

    std::set<int> savedMeshes;
    std::set<int> savedMaterials;
    std::set<int> savedTextures;

    size_t startMeshCount = GameEngine::AssetManager::numAssets<GameEngine::Mesh>();
    size_t startTextureCount = GameEngine::AssetManager::numAssets<GameEngine::Texture>();
    size_t startMaterialCount = GameEngine::AssetManager::numAssets<GameEngine::Material>();

    for (const auto &meshId: model.meshes) {
        GameEngine::AssetManager::createAsset<GameEngine::Mesh>();
    }

    for (const auto &imageId: model.images) {
        GameEngine::AssetManager::createAsset<GameEngine::Texture>();
    }
    size_t nextFreeTextureIndex = model.images.size();
    int fakeEmissiveTextureIndex = -1;

    for (const auto &materialsId: model.materials) {
        GameEngine::AssetManager::createAsset<GameEngine::Material>();
    }

    GameEngine::Scene scene;
    std::vector<GameEngine::Entity> entities;

    std::unordered_set<std::string> usedTextureNames;
    std::unordered_set<std::string> usedMaterialNames;
    std::unordered_set<std::string> usedMeshNames;

    for (const auto &nodeId: model.scenes[0].nodes) {
        auto &node = model.nodes[nodeId];
        std::string nodeName = node.name;
        if (nodeName.empty()) {
            nodeName = "node_" + std::to_string(nodeId);
        }
        auto entity = scene.createEntity(nodeName);
        auto &transform = entity.getComponent<GameEngine::TransformComponent>();

        for (auto i = 0; i < node.translation.size(); i++) {
            transform.localPosition[i] = static_cast<float>(node.translation[i]);
        }
        for (auto i = 0; i < node.rotation.size(); i++) {
            transform.localRotation[i] = static_cast<float>(node.rotation[i]);
        }
        for (auto i = 0; i < node.scale.size(); i++) {
            transform.localScale[i] = static_cast<float>(node.scale[i]);
        }

        if (!node.matrix.empty()) {
            // TODO: handle node matrix
            std::cout << "node has transform matrix is not supported yet." << std::endl;
            return;
        }

        auto &mesh = model.meshes[node.mesh];

        if (mesh.primitives.empty()) {
            std::cout << "mesh does not have any primitives" << std::endl;
            return;
        }

        if (mesh.primitives.size() > 1) {
            std::cout << "mesh has multiple primitives which is not supported yet" << std::endl;
            // TODO: just make one child for each primitive
            return;
        }

        auto &primitive = mesh.primitives[0];

        if (!savedMeshes.contains(node.mesh)) {
            savedMeshes.insert(node.mesh);
            std::string meshName = mesh.name;
            if (meshName.empty() || usedMeshNames.contains(meshName)) {
                meshName = "mesh_" + std::to_string(node.mesh);
            }
            usedMeshNames.insert(meshName);

            GameEngineTools::writeGLTFMeshPrimitiveToFile(model, primitive, meshName, outputDir.string(), GameEngine::AssetManager::getAsset<GameEngine::Mesh>(node.mesh + startMeshCount).assetUUID());
        }

        auto &renderer = entity.addComponent<GameEngine::PBRRendererComponent>(false);
        renderer.meshHandle = node.mesh + startMeshCount;
        renderer.materialHandle = primitive.material + startMaterialCount;

        auto &material = model.materials[primitive.material];

        int occlusionTextureIndex = material.occlusionTexture.index;
        int metallicRoughnessTextureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        int albedoTextureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        int normalTextureIndex = material.normalTexture.index;

        if (occlusionTextureIndex == -1) {
            std::cout << "missing occlusion texture which is not supported yet" << std::endl;
            return;
        }
        if (metallicRoughnessTextureIndex == -1) {
            std::cout << "missing metallic toughness texture which is not supported yet" << std::endl;
            return;
        }
        if (albedoTextureIndex == -1) {
            std::cout << "missing albedo texture which is not supported yet" << std::endl;
            return;
        }
        if (normalTextureIndex == -1) {
            std::cout << "missing normal texture which is not supported yet" << std::endl;
            return;
        }

        bool metallicRoughnessOcclusionAllInOne = occlusionTextureIndex == metallicRoughnessTextureIndex;
        if (!metallicRoughnessOcclusionAllInOne) {
            std::cout << "metallicRoughness and occlusion must be the same texture (for now)" << std::endl;
            //TODO: combine these textures if they are different (with cpu or gpu, idk)
            return;
        }

        int occlusionRoughnessMetallicTextureIndex = occlusionTextureIndex;

        int emissiveTextureIndex = material.emissiveTexture.index;
        if (emissiveTextureIndex == -1) {
            if (fakeEmissiveTextureIndex == -1) {
                fakeEmissiveTextureIndex = static_cast<int>(nextFreeTextureIndex);
                nextFreeTextureIndex++;
                savedTextures.insert(fakeEmissiveTextureIndex);

                GameEngine::AssetManager::createAsset<GameEngine::Texture>();

                std::string textureName = "texture_" + std::to_string(fakeEmissiveTextureIndex);
                auto &uuid = GameEngine::AssetManager::getAsset<GameEngine::Texture>(fakeEmissiveTextureIndex + startTextureCount).assetUUID();
                const uint8_t data[4] = {0, 0, 0, 255};
                GameEngineTools::writeFakeTexture(data, textureName, outputDir, uuid);
            }
            emissiveTextureIndex = fakeEmissiveTextureIndex;
        }

        std::unordered_map<int, bool> textureHasTransparency;

        auto writeTextureIfNeeded = [&](int textureIndex) {
            if (!savedTextures.contains(textureIndex)) {
                savedTextures.insert(textureIndex);

                auto &texture = model.textures[textureIndex];
                auto &image = model.images[texture.source];

                std::string textureName = texture.name;
                if (textureName.empty() || usedTextureNames.contains(textureName)) {
                    textureName = "texture_" + std::to_string(textureIndex);
                }
                usedTextureNames.insert(textureName);

                auto &uuid = GameEngine::AssetManager::getAsset<GameEngine::Texture>(texture.source + startTextureCount).assetUUID();
                bool hasTransparency = GameEngineTools::writeGLTFTextureImageFile(image, getFirstWordBeforeDot(textureName), outputDir, uuid);
                textureHasTransparency[textureIndex] = hasTransparency;
            }
        };

        if (!savedMaterials.contains(primitive.material)) {
            savedMaterials.insert(node.mesh);

            std::vector<std::string> texturesUUIDs;
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[albedoTextureIndex].source + startTextureCount).assetUUID());
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[normalTextureIndex].source + startTextureCount).assetUUID());
            texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[occlusionRoughnessMetallicTextureIndex].source + startTextureCount).assetUUID());
            if (emissiveTextureIndex < model.textures.size()) {
                texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(model.textures[emissiveTextureIndex].source + startTextureCount).assetUUID());
            } else {
                texturesUUIDs.push_back(GameEngine::AssetManager::getAsset<GameEngine::Texture>(emissiveTextureIndex + startTextureCount).assetUUID());
            }

            writeTextureIfNeeded(albedoTextureIndex);
            writeTextureIfNeeded(normalTextureIndex);
            writeTextureIfNeeded(occlusionRoughnessMetallicTextureIndex);
            writeTextureIfNeeded(emissiveTextureIndex);

            std::string materialName = material.name;
            if (materialName.empty() || usedMaterialNames.contains(materialName)) {
                materialName = "material_" + std::to_string(primitive.material);
            }
            usedMaterialNames.insert(materialName);
            std::ofstream outputFile(outputDir / (materialName + ".gematerial"), std::ios::out | std::ios::binary);
            if (!outputFile) {
                std::cerr << "Error: Could not open file for writing!" << std::endl;
                return;
            }

            outputFile << GameEngine::AssetManager::getAsset<GameEngine::Material>(primitive.material + startMaterialCount).assetUUID();

            uint32_t assetVersion = 0;
            outputFile.write(reinterpret_cast<char *>(&assetVersion), sizeof(assetVersion));

            nlohmann::json materialJSON;
            materialJSON["shader"]["uuid"] = PBR_SHADER_UUID; // hard coded shader for now
            materialJSON["textureUUIDs"] = texturesUUIDs;
            materialJSON["renderQueue"] = static_cast<uint8_t>(textureHasTransparency[albedoTextureIndex] ? GameEngine::RenderQueue::Transparent : GameEngine::RenderQueue::Opaque);

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

    auto prefabRootEntity = scene.createEntity(inputFileName);

    for (auto &entity: entities) {
        rootEntities.insert(entity.getRootEntity().enttHandle());
    }

    for (auto &rootEntity: rootEntities) {
        GameEngine::Entity entity(rootEntity, &scene);
        entity.setParent(prefabRootEntity);
    }

    nlohmann::json entityJSON = GameEngine::entityToJSON(prefabRootEntity);

    std::ofstream outputFile(outputDir / (inputFilePath.stem().string() + ".geprefab"), std::ios::out);
    outputFile << entityJSON.dump();
}

}
