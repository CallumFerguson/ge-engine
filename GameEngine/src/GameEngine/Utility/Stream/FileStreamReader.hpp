#pragma once

#include <fstream>
#include "StreamReader.hpp"

namespace GameEngine {

class FileStreamReader : public StreamReader {
public:
    explicit FileStreamReader(const std::filesystem::path &filePath);

    ~FileStreamReader() override;

    uint64_t getStreamPosition() override;

    void setStreamPosition(uint64_t position) override;

    void readData(uint8_t *destination, size_t size) override;

    uint64_t getStreamLength() override;

private:
    std::ifstream m_fileStream;

    uint64_t m_streamLength;
};

}
