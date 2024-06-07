#include "renderImGuiHeirarchy.hpp"

#include <string>
#include <imgui.h>
#include "Components.hpp"
#include "ScriptableEntity.hpp"

namespace GameEngine {

static entt::entity s_selectedEntity = entt::null;

void renderImGuiEntity(const entt::registry &registry, entt::entity entity) {
    auto entityName = registry.get<NameComponent>(entity).name.c_str();
    auto &transform = registry.get<TransformComponent>(entity);

    ImGuiTreeNodeFlags flags = 0;
    flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    if (transform.childrenENTTHandles().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (entity == s_selectedEntity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = ImGui::TreeNodeEx(entityName, flags);
    if (ImGui::IsItemClicked()) {
        s_selectedEntity = entity;
    }

    if (opened) {
        for (auto &child: transform.childrenENTTHandles()) {
            renderImGuiEntity(registry, child);
        }
        ImGui::TreePop();
    }
}

void renderImGuiEntityHierarchy(entt::registry &registry) {
    ImGuiIO &io = ImGui::GetIO();

    float displayWidth = io.DisplaySize.x;
    float displayHeight = io.DisplaySize.y;

    float windowWidth = 300.0f;
    float windowHeight = displayHeight / 2.0f;

    float windowPosX = displayWidth - windowWidth;

    ImGui::SetNextWindowPos(ImVec2(windowPosX, 0));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    registry.view<entt::entity>().each([&](auto entity) {
        auto &transform = registry.get<TransformComponent>(entity);
        if (transform.parentENTTHandle() == entt::null) {
            renderImGuiEntity(registry, entity);
        }
    });

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(windowPosX, windowHeight));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (s_selectedEntity != entt::null) {
        auto name = registry.get<NameComponent>(s_selectedEntity).name;
        ImGui::Text("%s", name.c_str());
        auto entityHandle = std::to_string(static_cast<uint64_t>(s_selectedEntity));
        ImGui::Text("entt handle: %s", entityHandle.c_str());

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

        for (auto &componentName: registry.get<InfoComponent>(s_selectedEntity).componentNames) {
            if (componentName == "NameComponent" || componentName == "TransformComponent") {
                continue;
            }

            if (componentName == "CameraComponent") {
                if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto &camera = registry.get<CameraComponent>(s_selectedEntity);
                    camera.onImGui();
                }
            } else if (componentName == "MeshRendererComponent") {
                if (ImGui::CollapsingHeader("MeshRendererComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto &meshRenderer = registry.get<MeshRendererComponent>(s_selectedEntity);
                    meshRenderer.onImGui();
                }
            } else if (componentName == "WebGPUMeshRendererDataComponent") {
                if (ImGui::CollapsingHeader("WebGPUMeshRendererDataComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
                }
            } else {
                if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
//                    ImGui::Text("inspector not configured for component");
                    std::cout << "TODO: automate the above stuff with a register component like register script" << std::endl;
                }
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
