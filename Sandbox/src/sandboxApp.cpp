#include "sandboxApp.hpp"

#include <memory>
#include "GameEngine.hpp"
#include "scripts/TestRenderer.hpp"
#include "scripts/TrackFramerate.hpp"
#include "scripts/ImGuiDemoWindow.hpp"

void runSandboxApp() {
    GameEngine::App app;

    GameEngine::Scene &scene = app.getActiveScene();

//    Entity entity = scene.createEntity();
//    entity.addComponent<NativeScriptComponent>().bind<TestScript>();

//    Entity entity2 = scene.createEntity("my entity");
//    entity2.addComponent<NativeScriptComponent>().bind<TestScript>();

    auto camera = scene.createEntity();
    camera.addComponent<GameEngine::CameraComponent>(90.0);

    auto unlitShader = std::make_shared<GameEngine::WebGPUShader>("shaders/unlit_color.wgsl");

    auto mesh = std::make_shared<GameEngine::Mesh>("assets/sphere.glb.asset");

    GameEngine::Entity renderingEntity = scene.createEntity();
    renderingEntity.addComponent<GameEngine::NativeScriptComponent>().bind<TestRenderer>(unlitShader, mesh);

    GameEngine::Entity trackFPS = scene.createEntity();
    trackFPS.addComponent<GameEngine::NativeScriptComponent>().bind<TrackFramerate>();

//    Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addComponent<NativeScriptComponent>().bind<ImGuiDemoWindow>();

    app.run();
}
