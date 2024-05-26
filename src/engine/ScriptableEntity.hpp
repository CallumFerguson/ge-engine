#pragma once

#include "Entity.hpp"

class ScriptableEntity {
public:
    template<typename T>
    T &getComponent() {
        return m_entity.getComponent<T>();
    }

private:
    Entity m_entity;
};
