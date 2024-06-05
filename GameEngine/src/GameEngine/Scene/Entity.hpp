#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "Scene.hpp"
#include "Components.hpp"

namespace GameEngine {

class Entity {
public:
    Entity() = default;

    Entity(entt::entity enttEntity, Scene *scene);

    template<typename T, typename... Args>
    T &addComponent(Args &&... args) {
        static_assert(!std::is_same<T, NativeScriptComponent>::value, "NativeScriptComponent cannot be directly added. Use addScript instead");
        T &component = m_scene->m_registry.emplace<T>(m_enttEntity, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    bool hasComponent() {
        return m_scene->m_registry.all_of<T>(m_enttEntity);
    }

    template<typename T>
    T &getComponent() const {
        return m_scene->m_registry.get<T>(m_enttEntity);
    }

    template<typename T>
    void removeComponent() {
        m_scene->m_registry.remove<T>(m_enttEntity);
    }

    template<typename T, typename... Args>
    T &addScript(Args &&... args) {
        auto &nsc = m_scene->m_registry.get_or_emplace<NativeScriptComponent>(m_enttEntity);
        return nsc.bind<T>(std::forward<Args>(args)...);
    }

    Scene *getScene();

    void setParent(const Entity &entity);

    Entity getRootEntity();

    glm::mat4 globalModelMatrix();

    entt::entity enttHandle();

//    bool operator==(const Entity &other) const {
//        return m_enttEntity == other.m_enttEntity;
//    }

private:
    entt::entity m_enttEntity = entt::null;
    Scene *m_scene = nullptr;
};

}
