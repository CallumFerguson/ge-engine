#include "Components.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "../Core/Window.hpp"

namespace GameEngine {

glm::mat4 TransformComponent::localModelMatrix() {
    return glm::translate(glm::mat4(1.0f), localPosition) * glm::mat4_cast(localRotation) * glm::scale(glm::mat4(1.0f), localScale);
}

entt::entity TransformComponent::parentENTTHandle() const {
    return m_parentENTTHandle;
}

const std::vector<entt::entity> &TransformComponent::childrenENTTHandles() const {
    return m_childrenENTTHandles;
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

}
