#include "Random.hpp"

#include <random>

namespace GameEngine {

static std::random_device rd;
static std::mt19937 mt(rd());

float Random::value() {
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(mt);
}

// TODO: get a library for true 128 bit randomness
std::string Random::uuid() {
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(mt);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(mt);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(mt);
    }
    ss << "-";
    ss << dis2(mt);
    for (i = 0; i < 3; i++) {
        ss << dis(mt);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(mt);
    };
    return ss.str();
}

}
