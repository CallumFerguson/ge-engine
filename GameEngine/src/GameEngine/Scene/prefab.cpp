#include "prefab.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include "Components.hpp"
#include "Scene.hpp"

namespace GameEngine {

json entityToJSON(Entity &entity) {
    json entityJson;

    entityJson["components"] = json::array();
    for (auto &componentName: entity.getComponent<InfoComponent>().componentNames) {
        json componentJSON;
        componentJSON["type"] = componentName;
        componentJSON["properties"] = entity.getComponentJSON(componentName);
        entityJson["components"].push_back(componentJSON);
    }

    entityJson["scripts"] = json::array();
    for (auto &scriptName: entity.getComponent<InfoComponent>().scriptNames) {
        json scriptJSON;
        scriptJSON["type"] = scriptName;
        scriptJSON["properties"] = entity.getScriptJSON(scriptName);
        entityJson["scripts"].push_back(scriptJSON);
    }

    entityJson["children"] = json::array();
    for (auto &childEntityHandle: entity.getComponent<TransformComponent>().childrenENTTHandles()) {
        auto childEntity = Entity(childEntityHandle, entity.getScene());
        entityJson["children"].push_back(entityToJSON(childEntity));
    }
    return entityJson;
}

bool validateEntityJSON(const json &entityJSON) {
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

void jsonToEntity(const json &entityJSON, entt::entity parentENTTHandle, Scene &scene) {
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
            std::cout << "unsupported component " << componentName << std::endl;
        }
    }

    for (const auto &scriptJSON: entityJSON["scripts"]) {
        auto scriptName = scriptJSON["type"].get<std::string>();
        if (scriptName == "TestRenderer") {
            
        } else if (scriptName == "Rotator") {

        } else {
            std::cout << "unsupported script " << scriptName << std::endl;
        }
    }

    for (const auto &childJSON: entityJSON["children"]) {
        jsonToEntity(childJSON, entity.enttHandle(), scene);
    }
}

}
