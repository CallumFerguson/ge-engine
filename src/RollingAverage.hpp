#pragma once

#include <queue>

class RollingAverage {
public:
    explicit RollingAverage(int numSamples = 100);

    void addSample(double sample);

    double average();

private:
    int numSamples;
    std::queue<double> samples;
    double total = 0;
    double _average = 0;
};
