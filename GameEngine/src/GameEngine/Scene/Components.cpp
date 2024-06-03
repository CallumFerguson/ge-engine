#include "Components.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "../Core/Window.hpp"

namespace GameEngine {

CameraComponent::CameraComponent(float fieldOfView) {
    m_fov = fieldOfView;
    m_aspectRatio = Window::mainWindow().aspectRatio();
    m_projection = glm::perspectiveRH_ZO(fieldOfView, m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
}

const glm::mat4 &CameraComponent::projection() {
    float aspectRatio = Window::mainWindow().aspectRatio();
    if (m_aspectRatio != aspectRatio) {
        m_aspectRatio = aspectRatio;
        m_projection = glm::perspectiveRH_ZO(m_fov, m_aspectRatio, m_nearClippingPlane, m_farClippingPlane);
    }
    return m_projection;
}

}
