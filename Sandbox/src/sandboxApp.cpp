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
    camera.getComponent<GameEngine::TransformComponent>().position[2] = 2.5;
//
    auto unlitShader = std::make_shared<GameEngine::WebGPUShader>("shaders/unlit_color.wgsl");

    auto mesh = std::make_shared<GameEngine::Mesh>("assets/sphere.glb.asset");

    GameEngine::Entity renderingEntity = scene.createEntity("ball 1");
    renderingEntity.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity.addScript<Rotator>();

    GameEngine::Entity renderingEntity2 = scene.createEntity("ball 2");
    renderingEntity2.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity2.getComponent<GameEngine::TransformComponent>().position[0] = 2.5;

    GameEngine::Entity trackFPS = scene.createEntity();
    trackFPS.addScript<TrackFramerate>();

//    Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addComponent<NativeScriptComponent>().bind<ImGuiDemoWindow>();

    app.run();
}
