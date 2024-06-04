#include "Scene.hpp"

#include "Entity.hpp"
#include "Components.hpp"
#include "ScriptableEntity.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"

namespace GameEngine {

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
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            if (!instanceFunctions.instantiated) {
                instanceFunctions.instantiated = true;
                instanceFunctions.instance->m_entity = Entity(entity, this);
                instanceFunctions.onStart(instanceFunctions.instance);
            }
        }
    });

    m_registry.view<NativeScriptComponent>().each([](auto &nsc) {
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            instanceFunctions.onUpdate(instanceFunctions.instance);
        }
    });

    m_registry.view<TransformComponent, CameraComponent>().each([](auto &transform, auto &camera) {
        WebGPURenderer::updateCameraDataBuffer(CameraComponent::transformToView(transform), camera.projection());
    });

    m_registry.view<NativeScriptComponent>().each([](auto &nsc) {
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            instanceFunctions.onImGui(instanceFunctions.instance);
        }
    });

    m_registry.view<NativeScriptComponent>().each([](auto &nsc) {
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            instanceFunctions.onCustomRenderPass(instanceFunctions.instance);
        }
    });

    WebGPURenderer::startMainRenderPass();

    m_registry.view<NativeScriptComponent>().each([](auto &nsc) {
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            instanceFunctions.onMainRenderPass(instanceFunctions.instance);
        }
    });

    WebGPURenderer::endMainRenderPass();
}

}
