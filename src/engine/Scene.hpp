#pragma once

#include <entt/entt.hpp>
#include <iostream>

class Entity;

class Scene {
public:
    Entity createEntity();

private:
    entt::registry m_registry;

    friend class Entity;
};
