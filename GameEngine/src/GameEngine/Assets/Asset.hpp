#pragma once

#include <string>

class Asset {
public:
    Asset() = default;

    explicit Asset(std::string assetUUID);

    std::string &assetUUID();

protected:
    std::string m_assetUUID;
};
