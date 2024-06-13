#pragma once

#include <chrono>
#include <string>

class TimingHelper {
public:
    explicit TimingHelper(std::string name, double thresholdMS = 0.0);

    ~TimingHelper();

    void stop();

private:
    std::string m_name;

    std::chrono::high_resolution_clock::time_point m_start;

    double m_thresholdMS;

    bool m_stopped = false;
};
