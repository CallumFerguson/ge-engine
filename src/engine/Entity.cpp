#include "Entity.hpp"
#include <entt/entt.hpp>

Entity::Entity(entt::entity enttEntity, Scene *scene) : m_enttEntity(enttEntity), m_scene(scene) {}
