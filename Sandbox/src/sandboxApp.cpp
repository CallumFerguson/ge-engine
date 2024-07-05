#include "sandboxApp.hpp"

#include "GameEngine.hpp"
#include "scripts/scripts.hpp"
#include "utility/utility.hpp"

void runSandboxApp() {
    registerScripts();
    registerComponents();

    GameEngine::App app("Sandbox");

    GameEngine::Scene &scene = app.getActiveScene();

    auto cameraPivotY = scene.createEntity("Camera Pivot");

    auto cameraPivotX = scene.createEntity("Camera Pivot");
    cameraPivotX.setParent(cameraPivotY);

    auto camera = scene.createEntity("Main Camera");
    camera.setParent(cameraPivotX);
    camera.getComponent<GameEngine::TransformComponent>().localPosition.z = 1.5f;
    camera.addComponent<GameEngine::CameraComponent>(65.0f);
    camera.addScript<CameraController>();

//    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/buikslotermeerplein_1k");
//    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/rosendal_plains_1_2k");
    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/overcast_soil_puresky_2k");
    GameEngine::WebGPURenderer::setEnvironmentMap(environmentMapHandle);

    GameEngine::Entity trackFPS = scene.createEntity("Track FPS");
    trackFPS.addScript<TrackFramerate>();

    {
        std::string assetPath = "assets/models/FlightHelmet";
        GameEngine::Entity prefabEntity = GameEngine::jsonToEntity(assetPath, scene);
        auto &transform = prefabEntity.getComponent<GameEngine::TransformComponent>();
        transform.localPosition.x = 0.3f;
        transform.localPosition.y = -0.35f;
    }

    {
        std::string assetPath = "assets/models/BoomBox";
        GameEngine::Entity prefabEntity = GameEngine::jsonToEntity(assetPath, scene);
        auto &transform = prefabEntity.getComponent<GameEngine::TransformComponent>();
        transform.localPosition.x = -0.3f;
        transform.localScale.x = 25.0f;
        transform.localScale.y = 25.0f;
        transform.localScale.z = 25.0f;
    }

//    GameEngine::Entity imGuiDemoWindow = scene.createEntity();
//    imGuiDemoWindow.addScript<ImGuiDemoWindow>();

    app.run();
}
