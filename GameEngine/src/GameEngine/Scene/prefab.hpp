#pragma once

#include <nlohmann/json.hpp>
#include "Entity.hpp"

namespace GameEngine {

nlohmann::json entityToJSON(Entity &entity);

void jsonToEntity(const nlohmann::json &entityJSON, Scene &scene);

void jsonToEntity(const nlohmann::json &entityJSON, entt::entity parentENTTHandle, Scene &scene);

}
