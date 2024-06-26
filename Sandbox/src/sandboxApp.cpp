#include "sandboxApp.hpp"

#include <memory>
#include "GameEngine.hpp"
#include "scripts/scripts.hpp"
#include "utility/utility.hpp"

void runSandboxApp() {
    registerScripts();
    registerComponents();

    GameEngine::App app;

    GameEngine::Scene &scene = app.getActiveScene();

    auto cameraPivotY = scene.createEntity("Camera Pivot");

    auto cameraPivotX = scene.createEntity("Camera Pivot");
    cameraPivotX.setParent(cameraPivotY);

    auto camera = scene.createEntity("Main Camera");
    camera.setParent(cameraPivotX);
    camera.getComponent<GameEngine::TransformComponent>().localPosition.z = 1.5f;
    camera.addComponent<GameEngine::CameraComponent>(65.0f);
    camera.addScript<CameraController>();

//    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/packaged/buikslotermeerplein_1k.geenvironmentmap");
//    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/packaged/rosendal_plains_1_2k.geenvironmentmap");
    int environmentMapHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::EnvironmentMap>("assets/packaged/overcast_soil_puresky_2k.geenvironmentmap");
    auto environmentMap = GameEngine::AssetManager::getAsset<GameEngine::EnvironmentMap>(environmentMapHandle);

    GameEngine::Entity skybox = scene.createEntity("Skybox");
    skybox.addComponent<GameEngine::Skybox>(environmentMap.skyboxCubeMapHandle());

    GameEngine::Entity trackFPS = scene.createEntity("Track FPS");
    trackFPS.addScript<TrackFramerate>();

    {
        nlohmann::json entityJSON;
        std::ifstream jsonFile("assets/packaged/FlightHelmet/FlightHelmet.geprefab");
        jsonFile >> entityJSON;
        GameEngine::Entity prefabEntity = GameEngine::jsonToEntity(entityJSON, scene);
        auto &transform = prefabEntity.getComponent<GameEngine::TransformComponent>();
        transform.localPosition.x = 0.3f;
        transform.localPosition.y = -0.35f;
    }

    {
        nlohmann::json entityJSON;
        std::ifstream jsonFile("assets/packaged/BoomBox/BoomBox.geprefab");
        jsonFile >> entityJSON;
        GameEngine::Entity prefabEntity = GameEngine::jsonToEntity(entityJSON, scene);
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
