#pragma once

#include "Entity.hpp"

namespace GameEngine {

class ScriptableEntity {
public:
    ScriptableEntity() = default;

    ~ScriptableEntity() = default;

    ScriptableEntity(const ScriptableEntity &) = delete;

    ScriptableEntity &operator=(const ScriptableEntity &) = delete;

    ScriptableEntity(ScriptableEntity &&) = delete;

    ScriptableEntity &operator=(ScriptableEntity &&) = delete;

    template<typename T, typename... Args>
    T &addComponent(Args &&... args) {
        T &component = m_entity.addComponent<T>(std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    bool hasComponent() {
        return m_entity.hasComponent<T>();
    }

    template<typename T>
    T &getComponent() {
        return m_entity.getComponent<T>();
    }

    template<typename T>
    void removeComponent() {
        m_entity.removeComponent<T>();
    }

    Entity &getEntity();

private:
    Entity m_entity;

    friend class Scene;
};

}
