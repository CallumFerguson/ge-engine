#pragma once

#include "Entity.hpp"

class ScriptableEntity {
public:
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

private:
    Entity m_entity;

    friend class Scene;
};
