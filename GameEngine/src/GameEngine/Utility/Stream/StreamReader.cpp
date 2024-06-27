#include "StreamReader.hpp"

namespace GameEngine {

void StreamReader::readString(std::string &s) {
    uint32_t length;
    readRaw(length);
    s.resize(length);
    readData(reinterpret_cast<const uint8_t *>(s.data()), length);
}

}
