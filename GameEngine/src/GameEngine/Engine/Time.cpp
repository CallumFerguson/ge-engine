#include "Time.hpp"

#include "Window.hpp"

static double s_startTime = -1;
static double s_lastTime = -1;
static double s_time;
static double s_deltaTime;

double Time::time() {
    return s_time;
}

double Time::deltaTime() {
    return s_deltaTime;
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
