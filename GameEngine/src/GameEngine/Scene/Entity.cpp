#include "Entity.hpp"

namespace GameEngine {

Entity::Entity(entt::entity enttEntity, Scene *scene) : m_enttEntity(enttEntity), m_scene(scene) {}

Scene *Entity::getScene() {
    return m_scene;
}

void Entity::setParent(const Entity &entity) {
    auto &transform = getComponent<TransformComponent>();
    transform.m_parentENTTHandle = entity.m_enttEntity;
}

glm::mat4 Entity::globalModelMatrix() {
    auto &transform = getComponent<TransformComponent>();
    glm::mat4 modelMatrix = transform.localModelMatrix();
    entt::entity parent = transform.parentENTTHandle();

    while (parent != entt::null) {
        auto &parentTransform = m_scene->m_registry.get<TransformComponent>(parent);
        modelMatrix = parentTransform.localModelMatrix() * modelMatrix;
        parent = parentTransform.parentENTTHandle();
    }

    return modelMatrix;
}

}
