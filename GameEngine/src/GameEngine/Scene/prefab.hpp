#pragma once

#include <nlohmann/json.hpp>
#include "Entity.hpp"

namespace GameEngine {

nlohmann::json entityToJSON(Entity &entity);

Entity jsonToEntity(const nlohmann::json &entityJSON, Scene &scene);

Entity jsonToEntity(const nlohmann::json &entityJSON, entt::entity parentENTTHandle, Scene &scene);

}
