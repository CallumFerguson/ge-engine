#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "Entity.hpp"

namespace GameEngine {

nlohmann::json entityToJSON(Entity &entity);

Entity jsonToEntity(const std::string &assetPath, Scene &scene);

Entity jsonToEntity(const nlohmann::json &entityJSON, Scene &scene);

Entity jsonToEntity(const nlohmann::json &entityJSON, entt::entity parentENTTHandle, Scene &scene);

}
