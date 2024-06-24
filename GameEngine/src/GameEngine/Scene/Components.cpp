#include "Components.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "../Core/Window.hpp"
#include "../Core/Exit.hpp"
#include "../Assets/AssetManager.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"
#include "../Rendering/CubeMap.hpp"

namespace GameEngine {

nlohmann::json NameComponent::toJSON() {
    nlohmann::json result;
    result["name"] = name;
    return result;
}

glm::mat4 TransformComponent::localModelMatrix(bool ignoreScale) const {
    if (ignoreScale) {
        return glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation);
    } else {
        return glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation) * glm::scale(glm::mat4(1.0f), localScale);
    }
}

entt::entity TransformComponent::parentENTTHandle() const {
    return m_parentENTTHandle;
}

const std::vector<entt::entity> &TransformComponent::childrenENTTHandles() const {
    return m_childrenENTTHandles;
}

nlohmann::json TransformComponent::toJSON() {
    nlohmann::json result;
    result["localPosition"] = {localPosition.x, localPosition.y, localPosition.z};
    result["localRotation"] = {localRotation.x, localRotation.y, localRotation.z, localRotation.w};
    result["localScale"] = {localScale.x, localScale.y, localScale.z};
    return result;
}

void TransformComponent::onImGuiInspector() {
    ImGui::DragFloat3("local position", &localPosition[0], 0.1f);

    static float s_floats[3] = {0, 0, 0};
    static float s_previousFloats[3] = {0, 0, 0};
    if (ImGui::DragFloat3("local rotation", s_floats, 0.1f)) {
        float floatsDelta[3];
        for (int i = 0; i < 3; i++) {
            floatsDelta[i] = s_floats[i] - s_previousFloats[i];
        }

        localRotation *= glm::angleAxis(glm::radians(floatsDelta[0]), glm::vec3(1.0f, 0.0f, 0.0f));
        localRotation *= glm::angleAxis(glm::radians(floatsDelta[1]), glm::vec3(0.0f, 1.0f, 0.0f));
        localRotation *= glm::angleAxis(glm::radians(floatsDelta[2]), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    for (int i = 0; i < 3; i++) {
        s_previousFloats[i] = s_floats[i];
    }
    if (!ImGui::IsItemActive()) {
        glm::vec3 eulerRotation = glm::eulerAngles(localRotation);
        eulerRotation = glm::degrees(eulerRotation);

        for (int i = 0; i < 3; i++) {
            s_floats[i] = eulerRotation[i];
        }
    }

    ImGui::DragFloat3("local scale", &localScale[0], 0.1f);
}

CameraComponent::CameraComponent(float fieldOfView) {
    m_fov = fieldOfView;
    m_aspectRatio = Window::mainWindow().aspectRatio();
    m_projection = glm::perspectiveRH_ZO(glm::radians(fieldOfView), m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
}

const glm::mat4 &CameraComponent::projection() {
    float aspectRatio = Window::mainWindow().aspectRatio();
    if (m_aspectRatio != aspectRatio) {
        m_aspectRatio = aspectRatio;
        m_projection = glm::perspectiveRH_ZO(glm::radians(m_fov), m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
    }
    return m_projection;
}

glm::mat4 CameraComponent::modelToView(const glm::mat4 &model) {
    auto cameraPosition = glm::vec3(model[3]);
    auto forward = glm::vec3(model * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
    auto up = glm::vec3(model * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    return glm::lookAt(cameraPosition, forward, up);
}

void CameraComponent::onImGuiInspector() {
    if (ImGui::SliderFloat("FOV", &m_fov, 0.001f, 179.0f)) {
        m_projection = glm::perspectiveRH_ZO(glm::radians(m_fov), m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
    }

    ImGui::DragFloat("Exposure", &exposure, 0.001f);
}

nlohmann::json CameraComponent::toJSON() {
    nlohmann::json result;
    result["fov"] = m_fov;
    result["nearClippingPlane"] = m_nearClippingPlane;
    result["farClippingPlane"] = m_farClippingPlane;
    result["exposure"] = exposure;
    return result;
}

PBRRendererComponent::PBRRendererComponent(bool initializeForRendering) : initializeForRendering(initializeForRendering) {
    if (initializeForRendering) {
        exitApp("initializeForRendering should always be false when using this");
    }
}

PBRRendererComponent::PBRRendererComponent(int meshHandle, int materialHandle) : meshHandle(meshHandle), materialHandle(materialHandle) {}

void PBRRendererComponent::onImGuiInspector() {
    ImGui::Text("Mesh handle: %d", meshHandle);
    if (ImGui::TreeNode("color")) {
        auto colorPtr = glm::value_ptr(color);
        ImGui::ColorPicker3("##color picker", colorPtr);
        ImGui::TreePop();
    }
}

nlohmann::json PBRRendererComponent::toJSON() {
    nlohmann::json result;
    result["mesh"]["uuid"] = AssetManager::getAsset<Mesh>(meshHandle).assetUUID();
    result["material"]["uuid"] = AssetManager::getAsset<Material>(materialHandle).assetUUID();
    return result;
}

PBRRendererComponent::PBRRendererComponent(const nlohmann::json &componentJSON) {
    meshHandle = AssetManager::getOrLoadAssetFromUUID<Mesh>(componentJSON["mesh"]["uuid"]);
    materialHandle = AssetManager::getOrLoadAssetFromUUID<Material>(componentJSON["material"]["uuid"]);
}

Skybox::Skybox(int cubeMapHandle) : cubeMapHandle(cubeMapHandle) {
    auto &device = WebGPURenderer::device();

    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<WebGPUShader>(SKYBOX_SHADER_UUID);
    auto &shader = GameEngine::AssetManager::getAsset<WebGPUShader>(shaderHandle);

    auto &cubeMap = GameEngine::AssetManager::getAsset<CubeMap>(cubeMapHandle);

    std::array<wgpu::BindGroupEntry, 3> entries;
    entries[0].binding = 0;
    entries[0].buffer = WebGPURenderer::cameraDataBuffer();

    entries[1].binding = 1;
    entries[1].sampler = WebGPURenderer::basicSampler();

    entries[2].binding = 2;
    entries[2].textureView = cubeMap.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor;
    bindGroupDescriptor.layout = shader.renderPipeline(true).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = entries.size();
    bindGroupDescriptor.entries = entries.data();

    m_bindGroup = device.CreateBindGroup(&bindGroupDescriptor);
}

const wgpu::BindGroup &Skybox::bindGroup() const {
    return m_bindGroup;
}

}
