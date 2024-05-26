#include "SandboxApp.hpp"
#include "../engine/Entity.hpp"
#include "../engine/Components.hpp"
#include "TestScript.hpp"

SandboxApp::SandboxApp() {
    Scene &scene = m_app.getActiveScene();

    Entity entity = scene.createEntity();
    entity.addComponent<NativeScriptComponent>().bind<TestScript>();

    Entity entity2 = scene.createEntity("my entity");
    entity2.addComponent<NativeScriptComponent>().bind<TestScript>();
}

void SandboxApp::run() {
    m_app.run();
}
