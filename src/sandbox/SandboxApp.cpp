#include "SandboxApp.hpp"

#include <memory>
#include <utility>
#include "../engine/Entity.hpp"
#include "../engine/Components.hpp"
#include "TestScript.hpp"
#include "TestRenderer.hpp"
#include "TrackFramerate.hpp"
#include "ImGuiDemoWindow.hpp"

SandboxApp::SandboxApp() {
    m_app.init();

    Scene &scene = m_app.getActiveScene();

//    Entity entity = scene.createEntity();
//    entity.addComponent<NativeScriptComponent>().bind<TestScript>();

//    Entity entity2 = scene.createEntity("my entity");
//    entity2.addComponent<NativeScriptComponent>().bind<TestScript>();

    auto unlitShader = std::make_shared<WebGPUShader>("shaders/unlit_color.wgsl");

    Entity renderingEntity = scene.createEntity();
    renderingEntity.addComponent<NativeScriptComponent>().bind<TestRenderer>(unlitShader);

    Entity trackFPS = scene.createEntity();
    trackFPS.addComponent<NativeScriptComponent>().bind<TrackFramerate>();

//    Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addComponent<NativeScriptComponent>().bind<ImGuiDemoWindow>();
}

void SandboxApp::run() {
    m_app.run();
}
