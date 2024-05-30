#include "sandboxApp.hpp"

#include <memory>
#include <utility>

#include "GameEngine.hpp"

//#include "../../src/engine/App.hpp"
//#include "../../src/engine/Entity.hpp"
//#include "../../src/engine/Components.hpp"
#include "scripts/TestScript.hpp"
#include "scripts/TestRenderer.hpp"
#include "scripts/TrackFramerate.hpp"
#include "scripts/ImGuiDemoWindow.hpp"

void runSandboxApp() {
    App app;

    Scene &scene = app.getActiveScene();

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

    app.run();
}
