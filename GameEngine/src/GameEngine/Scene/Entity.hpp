#pragma once

#include <entt/entt.hpp>
#include "Scene.hpp"

namespace GameEngine {

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
    bool hasComponent() {
        return m_scene->m_registry.all_of<T>(m_enttEntity);
    }

    template<typename T>
    T &getComponent() {
        return m_scene->m_registry.get<T>(m_enttEntity);
    }

    template<typename T>
    void removeComponent() {
        m_scene->m_registry.remove<T>(m_enttEntity);
    }

    Scene *getScene();

private:
    entt::entity m_enttEntity = entt::null;
    Scene *m_scene = nullptr;
};

}
