#include "sandboxApp.hpp"

#include <memory>
#include "GameEngine.hpp"
#include "scripts/TestRenderer.hpp"
#include "scripts/TrackFramerate.hpp"
#include "scripts/ImGuiDemoWindow.hpp"
#include "scripts/CameraController.hpp"
#include "scripts/Rotator.hpp"

void runSandboxApp() {
    GameEngine::App app;

    GameEngine::Scene &scene = app.getActiveScene();

    auto camera = scene.createEntity();
    camera.addComponent<GameEngine::CameraComponent>(90.0);
    camera.addScript<CameraController>();
    camera.getComponent<GameEngine::TransformComponent>().localPosition[2] = 2.5;

    auto unlitShader = std::make_shared<GameEngine::WebGPUShader>("shaders/unlit_color.wgsl");

    auto mesh = std::make_shared<GameEngine::Mesh>("assets/FlightHelmetPackaged/FlightHelmet.asset");

    float scale = 1;

    GameEngine::Entity renderingEntity = scene.createEntity("ball 1");
    renderingEntity.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity.addScript<Rotator>().speed = 180;
    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[0] = scale;
    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[1] = scale;
    renderingEntity.getComponent<GameEngine::TransformComponent>().localScale[2] = scale;

    GameEngine::Entity renderingEntity2 = scene.createEntity("ball 2");
    renderingEntity2.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity2.addScript<Rotator>().speed = 90;
    renderingEntity2.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
    renderingEntity2.setParent(renderingEntity);

    GameEngine::Entity renderingEntity3 = scene.createEntity("ball 3");
    renderingEntity3.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity3.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5f / scale;
    renderingEntity3.setParent(renderingEntity2);

    GameEngine::Entity trackFPS = scene.createEntity();
    trackFPS.addScript<TrackFramerate>();

//    Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addComponent<NativeScriptComponent>().bind<ImGuiDemoWindow>();

    std::filesystem::path inputFilePath("assets/FlightHelmet/FlightHelmet.gltf");

    tinygltf::TinyGLTF loader;

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
        return;
    }

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

    std::set < entt::entity > rootEntities;

    auto prefabRootEntity = scene.createEntity(inputFilePath.stem().string());

    for (auto &entity: entities) {
        rootEntities.insert(entity.getRootEntity().enttHandle());
    }

    for (auto &rootEntity: rootEntities) {
        GameEngine::Entity entity(rootEntity, &scene);
        entity.setParent(prefabRootEntity);
    }

    app.run();
}
