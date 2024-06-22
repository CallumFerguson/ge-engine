#include "Entity.hpp"

#include <utility>

namespace GameEngine {

static std::map<std::string, std::function<void(Entity &entity, const nlohmann::json &scriptJSON)>> s_addScriptFromStringFunctions;
static std::map<std::string, std::function<void(Entity &entity, const nlohmann::json &componentJSON)>> s_addComponentFromStringFunctions;

Entity::Entity(entt::entity enttEntity, Scene *scene) : m_enttEntity(enttEntity), m_scene(scene) {}

Scene *Entity::getScene() {
    return m_scene;
}

void Entity::setParent(Entity &parentEntity) {
    getComponent<TransformComponent>().m_parentENTTHandle = parentEntity.m_enttEntity;

    parentEntity.getComponent<TransformComponent>().m_childrenENTTHandles.push_back(m_enttEntity);
}

glm::mat4 Entity::globalModelMatrix(bool ignoreSelfScale) {
    auto &transform = getComponent<TransformComponent>();
    glm::mat4 modelMatrix = transform.localModelMatrix(ignoreSelfScale);
    entt::entity parent = transform.parentENTTHandle();

    while (parent != entt::null) {
        auto &parentTransform = m_scene->m_registry.get<TransformComponent>(parent);
        modelMatrix = parentTransform.localModelMatrix() * modelMatrix;
        parent = parentTransform.parentENTTHandle();
    }

    return modelMatrix;
}

Entity Entity::getRootEntity() {
    auto &transform = getComponent<TransformComponent>();
    entt::entity parent = transform.parentENTTHandle();
    entt::entity lastParent = m_enttEntity;

    while (parent != entt::null) {
        auto &parentTransform = m_scene->m_registry.get<TransformComponent>(parent);
        lastParent = parent;
        parent = parentTransform.parentENTTHandle();
    }

    return Entity(lastParent, m_scene);
}

entt::entity Entity::enttHandle() {
    return m_enttEntity;
}

nlohmann::json Entity::getComponentJSON(const std::string &componentName) {
    auto &info = getComponent<InfoComponent>();

    auto it = info.componentToJSONFunctions.find(componentName);

    if (it != info.componentToJSONFunctions.end()) {
        return it->second();
    } else {
        std::cout << "getComponentJSON did not find component with name " << componentName << std::endl;
        return {};
    }
}

bool Entity::hasComponentJSON(const std::string &componentName) {
    auto &info = getComponent<InfoComponent>();
    auto it = info.componentToJSONFunctions.find(componentName);
    return it != info.componentToJSONFunctions.end();
}

nlohmann::json Entity::getScriptJSON(const std::string &scriptName) {
    auto &info = getComponent<InfoComponent>();

    auto it = info.scriptToJSONFunctions.find(scriptName);

    if (it != info.scriptToJSONFunctions.end()) {
        return it->second();
    } else {
        std::cout << "getScriptJSON did not find component with name " << scriptName << std::endl;
        return {};
    }
}

bool Entity::hasScriptJSON(const std::string &scriptName) {
    auto &info = getComponent<InfoComponent>();
    auto it = info.scriptToJSONFunctions.find(scriptName);
    return it != info.scriptToJSONFunctions.end();
}

void Entity::addScript(const std::string &scriptName, const nlohmann::json &scriptJSON) {
    auto it = s_addScriptFromStringFunctions.find(scriptName);

    if (it != s_addScriptFromStringFunctions.end()) {
        it->second(*this, scriptJSON);
    } else {
        std::cout << "addScript did not find script with name " << scriptName << ". make sure to register the script using Entity::registerAddScriptFromStringFunction" << std::endl;
    }
}

void Entity::addComponent(const std::string &componentName, const nlohmann::json &componentJSON) {
    auto it = s_addComponentFromStringFunctions.find(componentName);

    if (it != s_addComponentFromStringFunctions.end()) {
        it->second(*this, componentJSON);
    } else {
        std::cout << "addComponent did not find component with name " << componentName << ". make sure to register the component using Entity::registerAddComponentFromStringFunction" << std::endl;
    }
}

void Entity::registerAddScriptFromStringFunction(const std::string &scriptName, std::function<void(Entity &entity, const nlohmann::json &scriptJSON)> addScriptFunction) {
    s_addScriptFromStringFunctions[scriptName] = std::move(addScriptFunction);
}

void Entity::registerAddComponentFromStringFunction(const std::string &componentName, std::function<void(Entity &entity, const nlohmann::json &componentJSON)> addComponentFunction) {
    s_addComponentFromStringFunctions[componentName] = std::move(addComponentFunction);
}

Entity Entity::getParent() {
    return {getComponent<TransformComponent>().m_parentENTTHandle, m_scene};
}

}
