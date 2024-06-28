#include "FileStreamWriter.hpp"

#include "../../Core/Exit.hpp"

namespace GameEngine {

FileStreamWriter::FileStreamWriter(const std::filesystem::path &filePath) {
    m_fileStream.open(filePath, std::ios::binary | std::ios::out);
    if (!m_fileStream.is_open()) {
        exitApp("FileStreamWriter failed to open file: " + filePath.string());
    }
}

FileStreamWriter::~FileStreamWriter() {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

uint64_t FileStreamWriter::getStreamPosition() {
    return m_fileStream.tellp();
}

void FileStreamWriter::setStreamPosition(uint64_t position) {
    m_fileStream.seekp(static_cast<std::streamoff>(position));
    if (!m_fileStream) {
        std::cout << "FileStreamWriter::setStreamPosition failed to set stream position" << std::endl;
    }
}

void FileStreamWriter::writeData(const uint8_t *data, size_t size) {
    m_fileStream.write(reinterpret_cast<const char *>(data), size);
    if (!m_fileStream) {
        std::cout << "FileStreamWriter::writeData failed to write data to file" << std::endl;
    }
}

}
