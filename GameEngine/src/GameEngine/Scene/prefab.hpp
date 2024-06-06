#pragma once

#include <nlohmann/json.hpp>
#include "Entity.hpp"

namespace GameEngine {

using json = nlohmann::json;

json entityToJson(Entity &entity);

void createPrefabFromEntity(Entity &entity);

void loadPrefab();

}
