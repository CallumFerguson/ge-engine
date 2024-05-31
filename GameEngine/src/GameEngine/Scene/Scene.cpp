#include "Scene.hpp"

#include "Entity.hpp"
#include "Components.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"

Entity Scene::createEntity(const std::string &name) {
    Entity entity(m_registry.create(), this);
    entity.addComponent<TransformComponent>();
    entity.addComponent<NameComponent>(name);
    return entity;
}

Entity Scene::createEntity() {
    return createEntity("New Entity");
}

void Scene::onUpdate() {
    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        if (!nsc.instance) {
            nsc.instantiate();
            nsc.instance->m_entity = Entity(entity, this);
            nsc.onStart(nsc.instance);
        }
    });

    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        nsc.onUpdate(nsc.instance);
    });

    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        nsc.onImGui(nsc.instance);
    });

    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        nsc.onCustomRenderPass(nsc.instance);
    });

    WebGPURenderer::startMainRenderPass();

    m_registry.view<NativeScriptComponent>().each([&](auto entity, auto &nsc) {
        nsc.onMainRenderPass(nsc.instance);
    });

    WebGPURenderer::endMainRenderPass();
}
