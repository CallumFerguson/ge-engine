#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"

Entity Scene::createEntity() {
    Entity entity(m_registry.create(), this);
    entity.addComponent<TransformComponent>();
    return entity;
}
