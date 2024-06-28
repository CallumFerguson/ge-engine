#pragma once

#include "StreamWriter.hpp"

namespace GameEngine {

class FileStreamWriter : public StreamWriter {
public:
    explicit FileStreamWriter(const std::filesystem::path &filePath);

    virtual ~FileStreamWriter();

    virtual uint64_t getStreamPosition() override;

    virtual void setStreamPosition(uint64_t position) override;

    virtual void writeData(const uint8_t *data, size_t size) override;

private:
    std::ofstream m_fileStream;
};

}
