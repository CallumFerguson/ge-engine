#pragma once

#include <entt/entt.hpp>
#include "Components.hpp"

namespace GameEngine {

class Entity;

class Scene {
public:
    Scene();

    ~Scene();

    Entity createEntity(const std::string &name);

    Entity createEntity();

    void onUpdate();

private:
    entt::registry m_registry;

    void onMeshRendererConstruct(entt::registry &, entt::entity entity);

    void onMeshRendererDestroy(entt::registry &, entt::entity entity);

    friend class Entity;
};

}
