#pragma once

#include <entt/entt.hpp>
#include <iostream>

class Entity;

class Scene {
public:
    Entity createEntity(const std::string &name);

    Entity createEntity();

    void onUpdate();

private:
    entt::registry m_registry;

    friend class Entity;
};
