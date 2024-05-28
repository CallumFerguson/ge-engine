#include "TrackFramerate.hpp"

#include "../engine/Time.hpp"
#include <cmath>
#include <iostream>

void TrackFramerate::onUpdate() {
    m_fpsRollingAverage.addSample(1.0 / Time::deltaTime());
    std::cout << static_cast<int>(std::round(m_fpsRollingAverage.average())) << "fps" << std::endl;
}
