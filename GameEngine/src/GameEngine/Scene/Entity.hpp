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
HAS_MEMBER_FUNCTION(onImGuiInspector, hasOnImGuiInspector)

class Entity {
public:
    Entity() = default;

    Entity(entt::entity enttEntity, Scene *scene);

    template<typename T, typename... Args>
    T &addComponent(Args &&... args) {
        static_assert(!std::is_same<T, NativeScriptComponent>::value, "NativeScriptComponent cannot be directly added. Use addScript instead");
        static_assert(!std::is_base_of<ScriptableEntity, T>::value, "It looks like T is a script. use addScript instead");

        if (m_scene->m_registry.all_of<T>(m_enttEntity)) {
            std::cout << "entity already has component!" << std::endl;
        }

        T &component = m_scene->m_registry.emplace<T>(m_enttEntity, std::forward<Args>(args)...);

        auto &info = getComponent<InfoComponent>();
        info.componentNames.push_back(component.objectName());
        if constexpr (hasToJSON<T>::value) {
            info.componentToJSONFunctions[component.objectName()] = [&]() -> nlohmann::json {
                return component.toJSON();
            };
        }
        if constexpr (hasOnImGuiInspector<T>::value) {
            info.componentToOnImGuiInspectorFunctions[component.objectName()] = [&]() {
                component.onImGuiInspector();
            };
        }

        return component;
    }

    void addComponent(const std::string &componentName, const nlohmann::json &componentJSON);

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

    void addScript(const std::string &scriptName, const nlohmann::json &scriptJSON);

    nlohmann::json getScriptJSON(const std::string &scriptName);

    bool hasScriptJSON(const std::string &scriptName);

    Scene *getScene();

    void setParent(Entity &parentEntity);

    Entity getParent();

    Entity getRootEntity();

    glm::mat4 globalModelMatrix(bool ignoreSelfScale = false);

    entt::entity enttHandle();

    static void registerAddScriptFromStringFunction(const std::string &scriptName, std::function<void(Entity &entity, const nlohmann::json &scriptJSON)> addScriptFunction);

    static void registerAddComponentFromStringFunction(const std::string &componentName, std::function<void(Entity &entity, const nlohmann::json &componentJSON)> addComponentFunction);

    template<typename T>
    static void registerAddScriptFromStringFunction(const std::string &scriptName) {
        registerAddScriptFromStringFunction(scriptName, [](GameEngine::Entity &entity, const nlohmann::json &scriptJSON) {
            entity.addScript<T>().initFromJSON(scriptJSON);
        });
    }

    template<typename T>
    static void registerAddComponentFromStringFunction(const std::string &componentName) {
        registerAddComponentFromStringFunction(componentName, [](GameEngine::Entity &entity, const nlohmann::json &componenetJSON) {
            entity.addComponent<T>(componenetJSON);
        });
    }

private:
    entt::entity m_enttEntity = entt::null;
    Scene *m_scene = nullptr;
};

}
