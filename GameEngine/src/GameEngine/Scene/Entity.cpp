#include "Entity.hpp"

namespace GameEngine {

Entity::Entity(entt::entity enttEntity, Scene *scene) : m_enttEntity(enttEntity), m_scene(scene) {}

Scene *Entity::getScene() {
    return m_scene;
}

void Entity::setParent(const Entity &parentEntity) {
    getComponent<TransformComponent>().m_parentENTTHandle = parentEntity.m_enttEntity;

    parentEntity.getComponent<TransformComponent>().m_childrenENTTHandles.push_back(m_enttEntity);
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

Entity Entity::getRootEntity() {
    auto &transform = getComponent<TransformComponent>();
    entt::entity parent = transform.parentENTTHandle();
    entt::entity lastParent = m_enttEntity;

    while (parent != entt::null) {
        auto &parentTransform = m_scene->m_registry.get<TransformComponent>(parent);
        lastParent = parent;
        parent = parentTransform.parentENTTHandle();
    }

    return Entity(lastParent, m_scene);
}

entt::entity Entity::enttHandle() {
    return m_enttEntity;
}

}
