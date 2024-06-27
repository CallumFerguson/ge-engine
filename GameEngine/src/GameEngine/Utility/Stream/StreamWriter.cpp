#include "StreamWriter.hpp"

namespace GameEngine {

void StreamWriter::writeString(const std::string &s) {
    writeRaw(static_cast<uint32_t>(s.size()));
    writeData(reinterpret_cast<const uint8_t *>(s.data()), s.size());
}

}
