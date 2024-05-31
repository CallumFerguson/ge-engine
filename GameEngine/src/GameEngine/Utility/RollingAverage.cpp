#include "RollingAverage.hpp"

#include <stdexcept>
#include "../Core/Exit.hpp"

namespace GameEngine {

RollingAverage::RollingAverage(int numSamples) : m_numSamples(numSamples) {
    if (numSamples < 1) {
        exitApp("numSamples should be larger than 0");
    }
}

void RollingAverage::addSample(double sample) {
    m_samples.push(sample);
    m_total += sample;
    if (m_samples.size() > m_numSamples) {
        double oldestSample = m_samples.front();
        m_total -= oldestSample;
        m_samples.pop();
    }
    m_average = m_total / m_samples.size();
}

double RollingAverage::average() const {
    return m_average;
}

}
