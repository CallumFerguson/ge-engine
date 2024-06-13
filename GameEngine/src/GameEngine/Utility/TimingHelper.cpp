#include "TimingHelper.hpp"

#include <utility>

TimingHelper::TimingHelper(std::string name, double thresholdMS)
        : m_name(std::move(name)), m_start(std::chrono::high_resolution_clock::now()), m_thresholdMS(thresholdMS) {}

TimingHelper::~TimingHelper() {
    stop();
}

void TimingHelper::stop() {
    if (m_stopped) {
        return;
    }
    m_stopped = true;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - m_start).count();
    if (duration >= m_thresholdMS) {
        std::cout << "Timing [" << m_name << "]: " << std::fixed << std::setprecision(3) << duration << "ms" << std::endl;
    }
}
