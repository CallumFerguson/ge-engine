#include "StreamReader.hpp"

namespace GameEngine {

void StreamReader::readString(std::string &s) {
    uint32_t length;
    readRaw(length);
    s.resize(length);
    readData(reinterpret_cast<uint8_t *>(s.data()), length);
}

void StreamReader::readUUID(std::string &uuid) {
    uuid.resize(36);
    readData(reinterpret_cast<uint8_t *>(uuid.data()), 36);
}

}
