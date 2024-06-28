#pragma once

#include <string>

class Asset {
public:
    Asset() = default;

    explicit Asset(std::string assetUUID);

    std::string &assetUUID();

    static std::filesystem::path appendAssetFileIfNeeded(const std::string &assetPath, const std::string &extension);

protected:
    std::string m_assetUUID;
};
