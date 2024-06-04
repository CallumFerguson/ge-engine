#pragma once

namespace GameEngine {

class Time {
public:
    static float time();

    static float deltaTime();

private:
    static void onUpdate();

    friend class App;
};

}
