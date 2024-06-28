#include "Asset.hpp"

#include <utility>

Asset::Asset(std::string assetUUID) : m_assetUUID(std::move(assetUUID)) {}

std::string &Asset::assetUUID() {
    return m_assetUUID;
}

std::filesystem::path Asset::appendAssetFileIfNeeded(const std::string &assetPath, const std::string &extension) {
    std::filesystem::path path = assetPath;
    if (path.has_extension()) {
        return assetPath;
    }

    return path / (path.stem().string() + extension);
}
