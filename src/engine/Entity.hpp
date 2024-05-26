#pragma once

#include "Scene.hpp"
#include <entt/entt.hpp>

class Entity {
public:
    Entity() = default;

    Entity(entt::entity enttEntity, Scene *scene);

    template<typename T, typename... Args>
    T &addComponent(Args &&... args) {
        T &component = m_scene->m_registry.emplace<T>(m_enttEntity, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    T &getComponent() {
        return m_scene->m_registry.get<T>(m_enttEntity);
    }

private:
    entt::entity m_enttEntity = entt::null;
    Scene *m_scene = nullptr;
};
