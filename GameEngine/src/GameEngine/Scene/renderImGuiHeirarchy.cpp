#include "renderImGuiHeirarchy.hpp"

#include <string>
#include <imgui.h>
#include "Components.hpp"
#include "ScriptableEntity.hpp"

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
    ImGuiIO &io = ImGui::GetIO();
    float displayWidth = io.DisplaySize.x;
    float displayHeight = io.DisplaySize.y;

    // Fixed width for the windows
    float windowWidth = 300.0f;  // Adjust this value to your desired width

    // Calculate heights for each window
    float windowHeight = displayHeight / 2.0f;

    float windowPosX = displayWidth - windowWidth;

    // First window
    ImGui::SetNextWindowPos(ImVec2(windowPosX, 0));  // Top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

//    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    registry.view<entt::entity>().each([&](auto entity) {
        auto &transform = registry.get<TransformComponent>(entity);
        if (transform.parentENTTHandle() == entt::null) {
            renderEntityNode(registry, entity);
        }
    });

    ImGui::End();

    // Second window
    ImGui::SetNextWindowPos(ImVec2(windowPosX, windowHeight));  // Below the first window
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

//    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (s_selectedEntity != entt::null) {
        auto name = registry.get<NameComponent>(s_selectedEntity).name;
        ImGui::Text("%s", name.c_str());
        ImGui::Text("entt handle: %d", (int) s_selectedEntity);

        ImGui::Separator();

        ImGui::Text("Components:");

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto &transform = registry.get<TransformComponent>(s_selectedEntity);

            ImGui::DragFloat3("local position", &transform.localPosition[0], 0.1f);

            glm::vec3 eulerRotation = glm::eulerAngles(transform.localRotation);
            eulerRotation = glm::degrees(eulerRotation);  // Convert to degrees for easier manipulation
            if (ImGui::DragFloat3("local rotation", &eulerRotation[0], 0.1f)) {
                eulerRotation = glm::radians(eulerRotation);  // Convert back to radians
                transform.localRotation = glm::quat(eulerRotation);
            }

            ImGui::DragFloat3("local scale", &transform.localScale[0], 0.1f);
        }

        if (registry.all_of<CameraComponent>(s_selectedEntity)) {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto &camera = registry.get<CameraComponent>(s_selectedEntity);
                camera.onImGui();
            }
        }

        ImGui::Separator();

        ImGui::Text("Scripts:");

        if (registry.all_of<NativeScriptComponent>(s_selectedEntity)) {
            auto &nsc = registry.get<NativeScriptComponent>(s_selectedEntity);
            int i = 0;
            for (auto &nscInstanceFunctions: nsc.instancesFunctions) {
                if (ImGui::CollapsingHeader(nscInstanceFunctions.instance->objectName(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    nscInstanceFunctions.instance->onImGuiInspector();
                }
                i++;
            }
        } else {
            ImGui::Text("none");
        }
    }

    ImGui::End();
}

}
