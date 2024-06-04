#include "Time.hpp"

#include "Window.hpp"

namespace GameEngine {

static double s_startTime = -1;
static double s_lastTime = -1;
static double s_time;
static double s_deltaTime;

float Time::time() {
    return static_cast<float>(s_time);
}

float Time::deltaTime() {
    return static_cast<float>(s_deltaTime);
}

void Time::onUpdate() {
    double windowTime = Window::getWindowTime();

    if (s_startTime < 0) {
        s_startTime = windowTime;
    }

    s_time = windowTime - s_startTime;

    if (s_lastTime < 0) {
        s_deltaTime = 1.0 / 60.0;
    } else {
        s_deltaTime = s_time - s_lastTime;
    }
    s_lastTime = s_time;
}

}
