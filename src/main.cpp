#include "engine/App.hpp"
#include "engine/Entity.hpp"
#include "engine/Components.hpp"
//#include  "rendering/backends/webgpu/webGPU.hpp"

#include <iostream>
#include <vector>

struct Test : public ScriptableEntity {
    int x = 5;

    void onUpdate() {
        getComponent<TransformComponent>();
        std::cout << "update x: " << x << std::endl;
    }

    void onFirstUpdate() {
        std::cout << "first update x: " << x << std::endl;
    }
};

int main() {
    App app;

    Scene &scene = app.getActiveScene();

    Entity entity = scene.createEntity();
    entity.addComponent<NativeScriptComponent>().bind<Test>();

    NativeScriptComponent nsc;
    nsc.bind<Test>();

    app.run();
}
