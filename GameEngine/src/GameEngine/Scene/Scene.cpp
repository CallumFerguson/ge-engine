#include "Scene.hpp"

#include <imgui.h>
#include "Entity.hpp"
#include "Components.hpp"
#include "ScriptableEntity.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"
#include "renderImGuiHeirarchy.hpp"

namespace GameEngine {

Scene::Scene() {
    m_registry.on_construct<MeshRendererComponent>().connect<&Scene::onMeshRendererConstruct>(this);
    m_registry.on_destroy<MeshRendererComponent>().connect<&Scene::onMeshRendererDestroy>(this);
}

Scene::~Scene() {
    m_registry.on_construct<MeshRendererComponent>().disconnect<&Scene::onMeshRendererConstruct>(this);
    m_registry.on_destroy<MeshRendererComponent>().disconnect<&Scene::onMeshRendererDestroy>(this);
}

Entity Scene::createEntity(const std::string &name) {
    Entity entity(m_registry.create(), this);
    m_registry.emplace<InfoComponent>(entity.enttHandle());
    entity.addComponent<NameComponent>(name);
    entity.addComponent<TransformComponent>();
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

    renderImGuiEntityHierarchy(m_registry);

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

    m_registry.view<MeshRendererComponent, WebGPUMeshRendererDataComponent>().each([&](auto enttEntity, auto &meshRenderer, auto &meshRendererData) {
        auto entity = Entity(enttEntity, this);
        WebGPURenderer::renderMesh(entity, meshRenderer, meshRendererData);
    });

    WebGPURenderer::endMainRenderPass();
}

void Scene::onMeshRendererConstruct(entt::registry &, entt::entity entity) {
    Entity(entity, this).addComponent<WebGPUMeshRendererDataComponent>();
}

void Scene::onMeshRendererDestroy(entt::registry &, entt::entity entity) {
//    Entity(entity, this).removeComponent<WebGPUMeshRendererDataComponent>();
    std::cout << "TODO: onMeshRendererDestroy" << std::endl;
}

}
