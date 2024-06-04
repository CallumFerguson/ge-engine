#include "renderImGuiHeirarchy.hpp"

#include <imgui.h>
#include "Components.hpp"

namespace GameEngine {

// super janky just for testing

static entt::entity s_selectedEntity = entt::null;

void renderEntityNode(const entt::registry &registry, entt::entity entity) {
    ImGuiTreeNodeFlags flags = ((s_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, "%s", registry.get<NameComponent>(entity).name.c_str());
    if (ImGui::IsItemClicked()) {
        s_selectedEntity = entity;
    }

    if (opened) {
        registry.view<entt::entity>().each([&](auto childEntity) {
            auto &childTransform = registry.get<TransformComponent>(childEntity);
            if (childTransform.parentENTTHandle() == entity) {
                renderEntityNode(registry, childEntity);
            }
        });
        ImGui::TreePop();
    }
}

void renderImGuiEntityHierarchy(const entt::registry &registry) {
    ImGui::Begin("Scene Hierarchy");

    registry.view<entt::entity>().each([&](auto entity) {
        auto &transform = registry.get<TransformComponent>(entity);
        if (transform.parentENTTHandle() == entt::null) {
            renderEntityNode(registry, entity);
        }
    });

    ImGui::End();
}

}
