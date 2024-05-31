#pragma once

#include <queue>

namespace GameEngine {

class RollingAverage {
public:
    explicit RollingAverage(int numSamples = 100);

    void addSample(double sample);

    [[nodiscard]] double average() const;

private:
    int m_numSamples;
    std::queue<double> m_samples;
    double m_total = 0;
    double m_average = 0;
};

}
