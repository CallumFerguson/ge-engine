#include "Time.hpp"

#include <chrono>
#include "Window.hpp"

namespace GameEngine {

static std::chrono::time_point<std::chrono::steady_clock> s_start;

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
        s_start = std::chrono::high_resolution_clock::now();
    }

    s_time = windowTime - s_startTime;

    if (s_lastTime < 0) {
        s_deltaTime = 1.0 / 60.0;
    } else {
        s_deltaTime = s_time - s_lastTime;
    }
    s_lastTime = s_time;
}

double Time::realTimeSinceStart() {
    if (s_startTime < 0) {
        return 0;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - s_start).count();
    return duration;
}

void Time::printRealTimeSinceStartMS() {
    if (s_startTime < 0) {
        std::cout << "Real time since start: 0ms" << std::endl;
        return;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(stop - s_start).count();
    std::cout << "Real time since start: " << std::fixed << std::setprecision(3) << duration << "ms" << std::endl;
}

}
