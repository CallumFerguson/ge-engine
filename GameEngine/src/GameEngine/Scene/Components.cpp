#include "Components.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "../Core/Window.hpp"

namespace GameEngine {

glm::mat4 TransformComponent::localModel() {
    return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
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
    return glm::inverse(glm::translate(glm::mat4(1.0f), transform.position) * glm::mat4_cast(transform.rotation));
}

}
