#include "Entity.hpp"

namespace GameEngine {

Entity::Entity(entt::entity enttEntity, Scene *scene) : m_enttEntity(enttEntity), m_scene(scene) {}

Scene *Entity::getScene() {
    return m_scene;
}

}
