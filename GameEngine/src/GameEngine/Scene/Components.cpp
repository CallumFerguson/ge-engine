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

glm::mat4 TransformComponent::localModelMatrix() {
    return glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation) * glm::scale(glm::mat4(1.0f), localScale);
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

glm::mat4 CameraComponent::transformToView(const TransformComponent &transform) {
    // ignore transform scale
    return glm::inverse(glm::translate(glm::mat4(1.0f), transform.localPosition) * glm::mat4_cast(transform.localRotation));
}

void CameraComponent::onImGui() {
    if (ImGui::SliderFloat("FOV", &m_fov, 0.001f, 179.0f)) {
        m_projection = glm::perspectiveRH_ZO(glm::radians(m_fov), m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
    }
}

nlohmann::json CameraComponent::toJSON() {
    nlohmann::json result;
    result["fov"] = m_fov;
    result["nearClippingPlane"] = m_nearClippingPlane;
    result["farClippingPlane"] = m_farClippingPlane;
    return result;
}

PBRRendererComponent::PBRRendererComponent(bool initializeForRendering) : initializeForRendering(initializeForRendering) {
    if (initializeForRendering) {
        exitApp("initializeForRendering should always be false when using this");
    }
}

PBRRendererComponent::PBRRendererComponent(int meshHandle, int materialHandle) : meshHandle(meshHandle), materialHandle(materialHandle) {}

void PBRRendererComponent::onImGui() {
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

    auto &cubeMap = GameEngine::AssetManager::getAsset<CubeMap>(shaderHandle);

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
