#pragma once

#include <string>

namespace GameEngine {

class StreamWriter {
public:
    virtual ~StreamWriter() = default;

    virtual uint64_t getStreamPosition() = 0;

    virtual void setStreamPosition(uint64_t position) = 0;

    virtual void writeData(const uint8_t *data, size_t size) = 0;

    template<typename T>
    void writeRaw(const T &type) {
        writeData(reinterpret_cast<const uint8_t *>(&type), sizeof(T));
    }

    void writeString(const std::string &s);

    void writeUUID(const std::string &uuid);
};

}
