#include "Random.hpp"

#include <random>

namespace GameEngine {

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

float Random::value() {
    return dist(mt);
}

}
