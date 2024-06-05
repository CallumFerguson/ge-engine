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

void renderImGuiEntityHierarchy(entt::registry &registry) {
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Scene Hierarchy");

    registry.view<entt::entity>().each([&](auto entity) {
        auto &transform = registry.get<TransformComponent>(entity);
        if (transform.parentENTTHandle() == entt::null) {
            renderEntityNode(registry, entity);
        }
    });

    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Entity Inspector");

    if (s_selectedEntity != entt::null) {
        auto name = registry.get<NameComponent>(s_selectedEntity).name;
        ImGui::Text("%s", name.c_str());
        ImGui::Separator();

        auto &transform = registry.get<TransformComponent>(s_selectedEntity);
        ImGui::Text("Transform");

        ImGui::DragFloat3("local position", &transform.localPosition[0], 0.1f);

        glm::vec3 eulerRotation = glm::eulerAngles(transform.localRotation);
        eulerRotation = glm::degrees(eulerRotation);  // Convert to degrees for easier manipulation
        if (ImGui::DragFloat3("local rotation", &eulerRotation[0], 0.1f)) {
            eulerRotation = glm::radians(eulerRotation);  // Convert back to radians
            transform.localRotation = glm::quat(eulerRotation);
        }

        ImGui::DragFloat3("local scale", &transform.localScale[0], 0.1f);
    }

    ImGui::End();

}

}
