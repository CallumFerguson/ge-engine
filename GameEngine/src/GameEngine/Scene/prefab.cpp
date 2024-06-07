#include "prefab.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include "Components.hpp"
#include "Scene.hpp"

namespace GameEngine {

nlohmann::json entityToJSON(Entity &entity) {
    nlohmann::json entityJson;

    entityJson["components"] = nlohmann::json::array();
    for (auto &componentName: entity.getComponent<InfoComponent>().componentNames) {
        if (componentName == "WebGPUPBRRendererDataComponent") {
            continue;
        }

        if (!entity.hasComponentJSON(componentName)) {
            std::cout << "entityToJSON component " << componentName << " does not have a to JSON function." << std::endl;
            return {};
        }

        nlohmann::json componentJSON;
        componentJSON["type"] = componentName;
        componentJSON["properties"] = entity.getComponentJSON(componentName);
        entityJson["components"].push_back(componentJSON);
    }

    entityJson["scripts"] = nlohmann::json::array();
    for (auto &scriptName: entity.getComponent<InfoComponent>().scriptNames) {
        if (!entity.hasScriptJSON(scriptName)) {
            std::cout << "entityToJSON script " << scriptName << " does not have a to JSON function." << std::endl;
            return {};
        }

        nlohmann::json scriptJSON;
        scriptJSON["type"] = scriptName;
        scriptJSON["properties"] = entity.getScriptJSON(scriptName);
        entityJson["scripts"].push_back(scriptJSON);
    }

    entityJson["children"] = nlohmann::json::array();
    for (auto &childEntityHandle: entity.getComponent<TransformComponent>().childrenENTTHandles()) {
        auto childEntity = Entity(childEntityHandle, entity.getScene());
        entityJson["children"].push_back(entityToJSON(childEntity));
    }
    return entityJson;
}

bool validateEntityJSON(const nlohmann::json &entityJSON) {
    if (!entityJSON.contains("components") || !entityJSON["components"].is_array()) {
        std::cout << "No components found." << std::endl;
        return false;
    }

    if (!entityJSON.contains("scripts") || !entityJSON["scripts"].is_array()) {
        std::cout << "No scripts found." << std::endl;
        return false;
    }

    if (!entityJSON.contains("children") || !entityJSON["children"].is_array()) {
        std::cout << "No children found." << std::endl;
        return false;
    }

    bool hasName = false;
    bool hasTransform = false;

    for (const auto &componentJSON: entityJSON["components"]) {
        if (componentJSON.contains("type")) {
            std::string componentName = componentJSON["type"].get<std::string>();
            if (componentName == "NameComponent") {
                hasName = true;
            }
            if (componentName == "TransformComponent") {
                hasTransform = true;
            }
        } else {
            std::cout << "component missing type" << std::endl;
            return false;
        }

        if (!componentJSON.contains("properties")) {
            std::cout << "component missing properties" << std::endl;
            return false;
        }
    }

    if (!hasName || !hasTransform) {
        std::cout << "missing name or transform" << std::endl;
        return false;
    }

    return true;
}

void jsonToEntity(const nlohmann::json &entityJSON, Scene &scene) {
    jsonToEntity(entityJSON, entt::null, scene);
}

void jsonToEntity(const nlohmann::json &entityJSON, entt::entity parentENTTHandle, Scene &scene) {
    if (!validateEntityJSON(entityJSON)) {
        std::cout << "entity is not valid" << std::endl;
        return;
    }

    std::string name;
    for (const auto &componentJSON: entityJSON["components"]) {
        if (componentJSON["type"].get<std::string>() == "NameComponent") {
            name = componentJSON["properties"]["name"].get<std::string>();
        }
    }

    if (name.empty()) {
        std::cout << "could not get name" << std::endl;
        return;
    }

    Entity entity = scene.createEntity(name);
    if (parentENTTHandle != entt::null) {
        Entity parent = Entity(parentENTTHandle, &scene);
        entity.setParent(parent);
    }

    for (const auto &componentJSON: entityJSON["components"]) {
        auto componentName = componentJSON["type"].get<std::string>();
        if (componentName == "NameComponent") {
            // handled at entity creation time
        } else if (componentName == "TransformComponent") {
            auto &transform = entity.getComponent<TransformComponent>();

            auto localPosition = componentJSON["properties"]["localPosition"].get<std::vector<float>>();
            transform.localPosition.x = localPosition[0];
            transform.localPosition.y = localPosition[1];
            transform.localPosition.z = localPosition[2];

            auto localRotation = componentJSON["properties"]["localRotation"].get<std::vector<float>>();
            transform.localRotation.x = localRotation[0];
            transform.localRotation.y = localRotation[1];
            transform.localRotation.z = localRotation[2];
            transform.localRotation.w = localRotation[3];

            auto localScale = componentJSON["properties"]["localScale"].get<std::vector<float>>();
            transform.localScale.x = localScale[0];
            transform.localScale.y = localScale[1];
            transform.localScale.z = localScale[2];
        } else {
            entity.addComponent(componentName);
        }
    }

    for (const auto &scriptJSON: entityJSON["scripts"]) {
        auto scriptName = scriptJSON["type"].get<std::string>();
        entity.addScript(scriptName, scriptJSON["properties"]);
    }

    for (const auto &childJSON: entityJSON["children"]) {
        jsonToEntity(childJSON, entity.enttHandle(), scene);
    }
}

}
