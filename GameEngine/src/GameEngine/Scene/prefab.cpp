#include "prefab.hpp"

#include <fstream>
#include <iostream>

namespace GameEngine {

json entityToJson(Entity &entity) {
    auto &transform = entity.getComponent<TransformComponent>();

    json entityJson;
    entityJson["components"] = {
            {
                    "type", "NameComponent",
                    "properties", {
                                          "name",     "Entity",
                                  }
            },
            {
                    "type", "TransformComponent",
                    "properties", {
                                          "position", {0, 0, 0},
                                          "rotation", {0, 0, 0, 1},
                                          "scale", {1, 1, 1},
                                  }
            }
    };
    entityJson["children"] = json::array();
    for (auto &childEntityHandle: transform.childrenENTTHandles()) {
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
