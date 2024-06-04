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

    auto mesh = std::make_shared<GameEngine::Mesh>("assets/sphere.glb.asset");

    GameEngine::Entity renderingEntity = scene.createEntity("ball 1");
    renderingEntity.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity.addScript<Rotator>().speed = 180;

    GameEngine::Entity renderingEntity2 = scene.createEntity("ball 2");
    renderingEntity2.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity2.addScript<Rotator>().speed = 90;
    renderingEntity2.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5;
    renderingEntity2.setParent(renderingEntity);

    GameEngine::Entity renderingEntity3 = scene.createEntity("ball 3");
    renderingEntity3.addScript<TestRenderer>(unlitShader, mesh);
    renderingEntity3.getComponent<GameEngine::TransformComponent>().localPosition[0] = 2.5;
    renderingEntity3.setParent(renderingEntity2);

    GameEngine::Entity trackFPS = scene.createEntity();
    trackFPS.addScript<TrackFramerate>();

//    Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addComponent<NativeScriptComponent>().bind<ImGuiDemoWindow>();

    app.run();
}
