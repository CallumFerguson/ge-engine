#pragma once

namespace GameEngine {

class Time {
public:
    static float time();

    static float deltaTime();

    static double realTimeSinceStart();

    static void printRealTimeSinceStartMS();

private:
    static void onUpdate();

    friend class App;
};

}
