#pragma once

#include <nlohmann/json.hpp>
#include "Entity.hpp"

namespace GameEngine {

using json = nlohmann::json;

json entityToJSON(Entity &entity);

void jsonToEntity(const json &entityJSON, entt::entity parentENTTHandle, Scene &scene);

}
