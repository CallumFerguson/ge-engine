#include "FileStreamReader.hpp"

#include "../../Core/Exit.hpp"

namespace GameEngine {

FileStreamReader::FileStreamReader(const std::filesystem::path &filePath) {
    m_fileStream.open(filePath, std::ios::binary | std::ios::ate);
    if (!m_fileStream.is_open()) {
        exitApp("FileStreamReader failed to open file: " + filePath.string());
    }

    m_streamLength = m_fileStream.tellg();
    m_fileStream.seekg(0, std::ios::beg);
}

FileStreamReader::~FileStreamReader() {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

uint64_t FileStreamReader::getStreamPosition() {
    return static_cast<uint64_t>(m_fileStream.tellg());
}

void FileStreamReader::setStreamPosition(uint64_t position) {
    m_fileStream.seekg(static_cast<std::streamoff>(position));
}

void FileStreamReader::readData(uint8_t *destination, size_t size) {
    m_fileStream.read(reinterpret_cast<char *>(destination), size);
    if (!m_fileStream) {
        std::cout << "FileStreamReader::readData failed to read data from file." << std::endl;
    }
}

uint64_t FileStreamReader::getStreamLength() {
    return m_streamLength;
}

}
