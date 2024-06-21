#include "sandboxApp.hpp"

#include <memory>
#include "GameEngine.hpp"
#include "scripts/scripts.hpp"
#include "utility/utility.hpp"

void runSandboxApp() {
    registerScripts();
    registerComponents();

    GameEngine::App app;

    GameEngine::Scene &scene = app.getActiveScene();

    auto camera = scene.createEntity("Main Camera");
    camera.addComponent<GameEngine::CameraComponent>(90.0);
    camera.addScript<CameraController>();
    camera.getComponent<GameEngine::TransformComponent>().localPosition.z = 1;

    int textureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>("assets/test.hdr");

    int cubeMapHandle = GameEngine::AssetManager::createAsset<GameEngine::CubeMap>(textureHandle);

    GameEngine::Entity skybox = scene.createEntity("Skybox");
    skybox.addComponent<GameEngine::Skybox>(cubeMapHandle);

//    auto fullscreen = scene.createEntity("Fullscreen");
//    fullscreen.addScript<FullscreenTexture>();

////    auto unlitShader = std::make_shared<GameEngine::WebGPUShader>("shaders/unlit_color.wgsl");
////    auto unlitShaderHandle = GameEngine::AssetManager::loadShader("shaders/unlit_color.wgsl");
//
////    auto mesh = std::make_shared<GameEngine::Mesh>("assets/FlightHelmetPackaged/FlightHelmet.gemesh");
//    auto meshHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Mesh>("assets/packaged/FlightHelmet/LeatherParts_low.gemesh");
////    auto meshHandle = GameEngine::AssetManager::getOrLoadMeshFromUUID("71cb6d06-21ea-43fb-991a-ac0ed33b16e2");
//
//    auto albedoTextureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>("assets/packaged/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_BaseColor.getexture");
//    auto normalTextureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>("assets/packaged/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_Normal.getexture");
//
//    auto shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::WebGPUShader>("assets/shaders/basic_color.wgsl");
//
//    int materialHandle = GameEngine::AssetManager::createAsset<GameEngine::Material>({});
//    auto &material = GameEngine::AssetManager::getAsset<GameEngine::Material>(materialHandle);
//    material.shaderHandle = shaderHandle;
//    material.addTexture(albedoTextureHandle);
//    material.addTexture(normalTextureHandle);
//    material.addTexture(albedoTextureHandle);
//    material.addTexture(albedoTextureHandle);
//    material.addTexture(albedoTextureHandle);
//    material.initBindGroup();
//
////    GameEngine::AssetManager::getOrLoad; // FlightHelmet_Materials_LeatherPartsMat_BaseColor.getexture
////    GameEngine::Texture texture("assets/packaged/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_BaseColor.getexture");
////    std::cout << texture.assetUUID() << std::endl;
//
//    float scale = 1;
//
//    GameEngine::Entity pbrRendererEntity1 = scene.createEntity("Thing 1");
//    pbrRendererEntity1.addComponent<GameEngine::PBRRendererComponent>(meshHandle, materialHandle);
//    pbrRendererEntity1.addScript<Rotator>().speed = 180;
//    pbrRendererEntity1.addScript<PBRColorRandomizer>();
//    pbrRendererEntity1.getComponent<GameEngine::TransformComponent>().localScale[0] = scale;
//    pbrRendererEntity1.getComponent<GameEngine::TransformComponent>().localScale[1] = scale;
//    pbrRendererEntity1.getComponent<GameEngine::TransformComponent>().localScale[2] = scale;

//    GameEngine::Entity pbrRendererEntity2 = scene.createEntity("Thing 2");
//    pbrRendererEntity2.addComponent<GameEngine::PBRRendererComponent>(meshHandle);
//    pbrRendererEntity2.addScript<Rotator>().speed = 90;
//    pbrRendererEntity2.addScript<PBRColorRandomizer>();
//    pbrRendererEntity2.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
//    pbrRendererEntity2.setParent(pbrRendererEntity1);
//
//    GameEngine::Entity pbrRendererEntity3 = scene.createEntity("Thing 3");
//    pbrRendererEntity3.addComponent<GameEngine::PBRRendererComponent>(meshHandle);
//    pbrRendererEntity3.addScript<PBRColorRandomizer>();
//    pbrRendererEntity3.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
//    pbrRendererEntity3.setParent(pbrRendererEntity2);

//    GameEngine::Entity renderingEntity = scene.createEntity("Thing 1");
//    renderingEntity.addScript<TestRenderer>(unlitShaderHandle, meshHandle);
//    renderingEntity.addScript<Rotator>().speed = 180;
//    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[0] = scale;
//    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[1] = scale;
//    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[2] = scale;
//
//    GameEngine::Entity renderingEntity2 = scene.createEntity("Thing 2");
//    renderingEntity2.addScript<TestRenderer>(unlitShaderHandle, meshHandle);
//    renderingEntity2.addScript<Rotator>().speed = 90;
//    renderingEntity2.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
//    renderingEntity2.setParent(renderingEntity);
//
//    GameEngine::Entity renderingEntity3 = scene.createEntity("Thing 3");
//    renderingEntity3.addScript<TestRenderer>(unlitShaderHandle, meshHandle);
//    renderingEntity3.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
//    renderingEntity3.setParent(renderingEntity2);

    GameEngine::Entity trackFPS = scene.createEntity("Track FPS");
    trackFPS.addScript<TrackFramerate>();

//    auto json = GameEngine::entityToJSON(pbrRendererEntity1);
//    GameEngine::jsonToEntity(json, entt::null, scene);

    nlohmann::json entityJSON;
    std::ifstream jsonFile("assets/packaged/FlightHelmet/FlightHelmet.geprefab");
    jsonFile >> entityJSON;
    GameEngine::Entity prefabEntity = GameEngine::jsonToEntity(entityJSON, scene);
    prefabEntity.getComponent<GameEngine::TransformComponent>().localPosition.y = -0.55f;

//    GameEngine::Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addScript<ImGuiDemoWindow>();

//    std::filesystem::path inputFilePath("assets/FlightHelmet/FlightHelmet.gltf");
//
//    tinygltf::TinyGLTF loader;
//
//    tinygltf::Model model;
//    std::string err;
//    std::string warn;
//
//    std::string inputFilePathExtension = inputFilePath.extension().string();
//    bool result;
//    if (inputFilePathExtension == ".gltf") {
//        result = loader.LoadASCIIFromFile(&model, &err, &warn, inputFilePath.string());
//    } else if (inputFilePathExtension == ".glb") {
//        result = loader.LoadBinaryFromFile(&model, &err, &warn, inputFilePath.string());
//    } else {
//        std::cout << "unknown file extension: " << inputFilePathExtension << " extension must be either .gltf or .glb" << std::endl;
//        return;
//    }
//
//    if (!warn.empty()) {
//        std::cout << "loadModel warning: " << warn << std::endl;
//    }
//    if (!err.empty()) {
//        std::cerr << "loadModel error: " << err << std::endl;
//    }
//    if (!result) {
//        std::cerr << "loadModel ailed to load GLTF model." << std::endl;
//        return;
//    }
//    if (model.scenes.size() != 1) {
//        // TODO: probably just make an empty root element
//        std::cout << "only gltf files with a single scene are supported. this gltf has " << model.scenes.size() << " scenes." << std::endl;
//        return;
//    }
//
////    json prefab;
////    prefab["components"] = {
////            {
////                    "type", "NameComponent",
////                    "properties", {
////                                          "name",     "Entity",
////                                  }
////            },
////            {
////                    "type", "TransformComponent",
////                    "properties", {
////                                          "position", {0, 0, 0},
////                                          "rotation", {0, 0, 0, 1},
////                                          "scale", {1, 1, 1},
////                                  }
////            }
////    };
////    prefab["children"] = json::array();
//
//    std::vector<GameEngine::Entity> entities;
//
//    for (const auto &nodeId: model.scenes[0].nodes) {
//        auto &node = model.nodes[nodeId];
//        auto entity = scene.createEntity(node.name);
//        entities.push_back(entity);
//    }
//
//    for (const auto &nodeId: model.scenes[0].nodes) {
//        auto &node = model.nodes[nodeId];
//        for (const auto &childNodeId: node.children) {
//            entities[childNodeId].setParent(entities[nodeId]);
//            std::cout << "set parent" << std::endl;
//
//        }
//    }
//
//    std::set<entt::entity> rootEntities;
//
//    auto prefabRootEntity = scene.createEntity(inputFilePath.stem().string());
//
//    for (auto &entity: entities) {
//        rootEntities.insert(entity.getRootEntity().enttHandle());
//    }
//
//    for (auto &rootEntity: rootEntities) {
//        GameEngine::Entity entity(rootEntity, &scene);
//        entity.setParent(prefabRootEntity);
//    }

    app.run();
}
