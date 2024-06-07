#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include "Scene.hpp"
#include "Components.hpp"

namespace GameEngine {

HAS_MEMBER_FUNCTION(toJSON, hasToJSON)

class Entity {
public:
    Entity() = default;

    Entity(entt::entity enttEntity, Scene *scene);

    template<typename T, typename... Args>
    T &addComponent(Args &&... args) {
        static_assert(!std::is_same<T, NativeScriptComponent>::value, "NativeScriptComponent cannot be directly added. Use addScript instead");
        static_assert(!std::is_base_of<ScriptableEntity, T>::value, "It looks like T is a script. use addScript instead");
        T &component = m_scene->m_registry.emplace<T>(m_enttEntity, std::forward<Args>(args)...);

        auto &info = getComponent<InfoComponent>();
        info.componentNames.push_back(component.objectName());
        if constexpr (hasToJSON<T>::value) {
            info.componentToJSONFunctions[component.objectName()] = [&]() -> nlohmann::json {
                return component.toJSON();
            };
        }

        return component;
    }

    void addComponent(const std::string &componentName) {
        // move this to cpp
        // use static set of functions that are registered before app creation in sandbox of wherever
        // and engine components can be automatically registered instead of defined in prefab.cpp
        std::cout << "TODO" << std::endl;
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

    bool hasComponentJSON(const std::string &componentName);

//    template<typename T>
//    void removeComponent() {
////        m_scene->m_registry.remove<T>(m_enttEntity);
//        std::cout << "TODO: implement removeComponent" << std::endl;
//    }

    template<typename T, typename... Args>
    T &addScript(Args &&... args) {
        static_assert(std::is_base_of<ScriptableEntity, T>::value, "T must inherit from ScriptableEntity");

        auto &nsc = m_scene->m_registry.get_or_emplace<NativeScriptComponent>(m_enttEntity);
        T &script = nsc.bind<T>(std::forward<Args>(args)...);

        auto &info = getComponent<InfoComponent>();
        info.scriptNames.push_back(script.objectName());
        if constexpr (hasToJSON<T>::value) {
            info.scriptToJSONFunctions[script.objectName()] = [&]() -> nlohmann::json {
                return script.toJSON();
            };
        }

        return script;
    }

    void addScript(const std::string &scriptName);

    nlohmann::json getScriptJSON(const std::string &scriptName);

    bool hasScriptJSON(const std::string &scriptName);

    Scene *getScene();

    void setParent(Entity &parentEntity);

    Entity getRootEntity();

    glm::mat4 globalModelMatrix();

    entt::entity enttHandle();

    static void registerAddScriptFromStringFunction(const std::string &scriptName, std::function<void(Entity *entity)> addScriptFunction);

private:
    entt::entity m_enttEntity = entt::null;
    Scene *m_scene = nullptr;
};

}
