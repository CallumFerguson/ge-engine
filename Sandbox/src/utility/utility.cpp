#include "utility.hpp"

#include "GameEngine.hpp"
#include "../scripts/scripts.hpp"

void registerScripts() {
    GameEngine::Entity::registerAddScriptFromStringFunction<CameraController>("CameraController");
    GameEngine::Entity::registerAddScriptFromStringFunction<ImGuiDemoWindow>("ImGuiDemoWindow");
    GameEngine::Entity::registerAddScriptFromStringFunction<Rotator>("Rotator");
    GameEngine::Entity::registerAddScriptFromStringFunction<TestRenderer>("TestRenderer");
    GameEngine::Entity::registerAddScriptFromStringFunction<TrackFramerate>("TrackFramerate");
    GameEngine::Entity::registerAddScriptFromStringFunction<PBRColorRandomizer>("PBRColorRandomizer");
}

void registerComponents() {

}
