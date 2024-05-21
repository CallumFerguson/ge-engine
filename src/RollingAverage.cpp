#include "RollingAverage.hpp"

RollingAverage::RollingAverage(int numSamples) {
    if (numSamples < 1) {
        throw std::runtime_error("numSamples should be larger than 0");
    }
    this->numSamples = numSamples;
}

void RollingAverage::addSample(double sample) {
    samples.push(sample);
    total += sample;
    if (samples.size() > numSamples) {
        double oldestSample = samples.front();
        total -= oldestSample;
        samples.pop();
    }
    _average = total / samples.size();
}

double RollingAverage::average() {
    return _average;
}
