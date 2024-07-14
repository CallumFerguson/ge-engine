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

    void showSceneHierarchy();

    void hideSceneHierarchy();

private:
    entt::registry m_registry;

    bool m_showSceneHierarchy = false;

    void onPBRRendererConstruct(entt::registry &, entt::entity entity);

    void onPBRRendererDestroy(entt::registry &, entt::entity entity);

    friend class Entity;
};

}
