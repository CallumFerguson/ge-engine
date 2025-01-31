#include "Scene.hpp"

#include <imgui.h>
#include "Entity.hpp"
#include "Components.hpp"
#include "ScriptableEntity.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"
#include "renderImGuiHeirarchy.hpp"

namespace GameEngine {

Scene::Scene() {
    m_registry.on_construct<PBRRendererComponent>().connect<&Scene::onPBRRendererConstruct>(this);
    m_registry.on_destroy<PBRRendererComponent>().connect<&Scene::onPBRRendererDestroy>(this);
}

Scene::~Scene() {
    m_registry.on_construct<PBRRendererComponent>().disconnect<&Scene::onPBRRendererConstruct>(this);
    m_registry.on_destroy<PBRRendererComponent>().disconnect<&Scene::onPBRRendererDestroy>(this);
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

    m_registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto &transform, auto &camera) {
        Entity e(entity, this);
        WebGPURenderer::updateCameraDataBuffer(e, transform, camera);
    });

    m_registry.view<NativeScriptComponent>().each([](auto &nsc) {
        for (NativeScriptComponent::NSCInstanceFunctions &instanceFunctions: nsc.instancesFunctions) {
            instanceFunctions.onImGui(instanceFunctions.instance);
        }
    });

    if (m_showSceneHierarchy) {
        renderImGuiEntityHierarchy(m_registry);
    }

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

    m_registry.view<PBRRendererComponent, WebGPUPBRRendererDataComponent>().each([&](auto enttEntity, auto &renderer, auto &rendererData) {
        auto entity = Entity(enttEntity, this);
        WebGPURenderer::submitMeshToRenderer(entity, renderer, rendererData);
    });

    WebGPURenderer::drawMainRenderPass();

    WebGPURenderer::endMainRenderPass();
}

void Scene::onPBRRendererConstruct(entt::registry &, entt::entity entity) {
    auto &renderer = m_registry.get<PBRRendererComponent>(entity);
    if (renderer.initializeForRendering) {
        Entity(entity, this).addComponent<WebGPUPBRRendererDataComponent>(renderer.materialHandle);
    }
}

void Scene::onPBRRendererDestroy(entt::registry &, entt::entity entity) {
    if (m_registry.get<PBRRendererComponent>(entity).initializeForRendering) {
//        Entity(entity, this).removeComponent<WebGPUPBRRendererDataComponent>();
        std::cout << "TODO: onPBRRendererDestroy" << std::endl;
    }
}

void Scene::showSceneHierarchy() {
    m_showSceneHierarchy = true;
}

void Scene::hideSceneHierarchy() {
    m_showSceneHierarchy = false;
}

}
