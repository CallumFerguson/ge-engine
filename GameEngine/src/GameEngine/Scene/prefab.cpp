#include "prefab.hpp"

#include <fstream>
#include <iostream>
#include "Components.hpp"

namespace GameEngine {

json entityToJson(Entity &entity) {
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
        entityJson["children"].push_back(entityToJson(childEntity));
    }
    return entityJson;
}

void createPrefabFromEntity(Entity &entity) {
    auto result = entityToJson(entity);

    std::ofstream outputFile("C:\\Users\\Calxf\\Documents\\CallumDocs\\prefab.json", std::ios::out);
    outputFile << result.dump(2);
}

void loadPrefab() {

}

}
