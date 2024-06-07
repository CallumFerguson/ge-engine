#pragma once

#include "GameEngine.hpp"

class PBRColorRandomizer : public GameEngine::ScriptableEntity {
public:
    void onStart();

    void onUpdate();

    nlohmann::json toJSON();

    void initFromJSON(const nlohmann::json &scriptJSON);

    [[nodiscard]] const char *objectName() const override {
        return "PBRColorRandomizer";
    }

private:
    void randomizeColor();
};
