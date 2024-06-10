#include "Asset.hpp"

#include <utility>

Asset::Asset(std::string assetUUID) : m_assetUUID(std::move(assetUUID)) {}

std::string &Asset::assetUUID() {
    return m_assetUUID;
}
