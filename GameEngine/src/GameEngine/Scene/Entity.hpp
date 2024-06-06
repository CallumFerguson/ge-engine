#pragma once

#include <vector>
#include <map>
#include <functional>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
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

        auto &info = getComponent<InfoComponent>();
        info.componentNames.push_back(component.objectName());
        info.componentToJSONFunctions[component.objectName()] = [&]() -> nlohmann::json {
            return component.toJSON();
        };

        return component;
    }

    template<typename T>
    bool hasComponent() {
        return m_scene->m_registry.all_of<T>(m_enttEntity);
    }

    template<typename T>
    [[nodiscard]] T &getComponent() const {
        return m_scene->m_registry.get<T>(m_enttEntity);
    }

    nlohmann::json getComponentJSON(const std::string &componentName);

    template<typename T>
    void removeComponent() {
        m_scene->m_registry.remove<T>(m_enttEntity);
    }

    template<typename T, typename... Args>
    T &addScript(Args &&... args) {
        auto &nsc = m_scene->m_registry.get_or_emplace<NativeScriptComponent>(m_enttEntity);
        T &script = nsc.bind<T>(std::forward<Args>(args)...);

        auto &info = getComponent<InfoComponent>();
        info.scriptNames.push_back(script.objectName());
        info.scriptToJSONFunctions[script.objectName()] = [&]() -> nlohmann::json {
            return script.toJSON();
        };

        return script;
    }

    nlohmann::json getScriptJSON(const std::string &scriptName);

    Scene *getScene();

    void setParent(Entity &parentEntity);

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
