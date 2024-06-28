#include "StreamWriter.hpp"

namespace GameEngine {

void StreamWriter::writeString(const std::string &s) {
    writeRaw(static_cast<uint32_t>(s.size()));
    writeData(reinterpret_cast<const uint8_t *>(s.data()), s.size());
}

void StreamWriter::writeUUID(const std::string &uuid) {
    if (uuid.size() != 36) {
        std::cout << "writeUUID uuid " << uuid << " is not a valid uuid" << std::endl;
        return;
    }
    writeData(reinterpret_cast<const uint8_t *>(uuid.data()), 36);
}

}
