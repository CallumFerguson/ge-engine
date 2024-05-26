#include "engine/App.hpp"
#include "engine/Entity.hpp"
#include "engine/Components.hpp"
//#include  "rendering/backends/webgpu/webGPU.hpp"

#include <iostream>
#include <vector>

struct Test : public ScriptableEntity {
    int x = 5;

    void onFirstUpdate() {
        auto &transform = getComponent<TransformComponent>();
        std::cout << "first update x: " << x << ", " << transform.position[0] << std::endl;
    }

    void onUpdate() {
        auto &transform = getComponent<TransformComponent>();
        transform.position[0] += 1;
        std::cout << "update x: " << x << ", " << transform.position[0] << ", name: "
                  << getComponent<NameComponent>().name << std::endl;
    }
};

int main() {
    App app;

    Scene &scene = app.getActiveScene();

    Entity entity = scene.createEntity();
    entity.addComponent<NativeScriptComponent>().bind<Test>();

    Entity entity2 = scene.createEntity("my entity");
    entity2.addComponent<NativeScriptComponent>().bind<Test>();

    app.run();
}
