#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"

Entity Scene::createEntity(const std::string &name) {
    Entity entity(m_registry.create(), this);
    entity.addComponent<TransformComponent>();
    entity.addComponent<NameComponent>(name);
    return entity;
}

Entity Scene::createEntity() {
    return createEntity("New Entity");
}

struct Test;

void Scene::onUpdate() {
    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        if (!nsc.instance) {
            nsc.instantiate();
            nsc.instance->m_entity = Entity(entity, this);
            nsc.onFirstUpdate(nsc.instance);
        }

        nsc.onUpdate(nsc.instance);
    });
}
