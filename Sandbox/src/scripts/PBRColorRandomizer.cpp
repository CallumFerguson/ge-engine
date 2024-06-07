#include "PBRColorRandomizer.hpp"

void PBRColorRandomizer::onStart() {
    auto &color = getComponent<GameEngine::PBRRendererComponent>().color;

    color[0] = 0.25f;
    color[1] = 0.0f;
    color[2] = 0.0f;
}

void PBRColorRandomizer::onUpdate() {
    if (GameEngine::Input::getKeyDown(GameEngine::KeyCode::Space)) {
        randomizeColor();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::R)) {
        randomizeColor();
    }
}

void PBRColorRandomizer::randomizeColor() {
    auto &color = getComponent<GameEngine::PBRRendererComponent>().color;

    color[0] = GameEngine::Random::value();
    color[1] = GameEngine::Random::value();
    color[2] = GameEngine::Random::value();
}

nlohmann::json PBRColorRandomizer::toJSON() {
    return {};
}

void PBRColorRandomizer::initFromJSON(const nlohmann::json &scriptJSON) {
}
