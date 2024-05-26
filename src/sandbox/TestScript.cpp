#include "TestScript.hpp"

void TestScript::onFirstUpdate() {
    auto &transform = getComponent<TransformComponent>();
    std::cout << "first update: " << transform.position[0] << std::endl;
}

void TestScript::onUpdate() {
    auto &transform = getComponent<TransformComponent>();
    transform.position[0] += 1;
    std::cout << "update: " << transform.position[0] << ", name: "
              << getComponent<NameComponent>().name << std::endl;
}
