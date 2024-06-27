#pragma once

#include <string>

namespace GameEngine {

class StreamReader {
public:
    virtual ~StreamReader() = default;

    virtual uint64_t getStreamPosition() = 0;

    virtual void setStreamPosition(uint64_t position) = 0;

    virtual void readData(const uint8_t *destination, size_t size) = 0;

    template<typename T>
    void readRaw(const T &type) {
        readData(reinterpret_cast<const uint8_t *>(&type), sizeof(T));
    }

    void readString(std::string &s);
};

}