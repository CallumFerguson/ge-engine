template<typename T>
std::vector<T> AssetManager::AssetData<T>::assets;

extern std::unordered_map<std::string, int> g_AssetManagerAssetPathToHandle;
extern std::unordered_map<std::string, int> g_AssetManagerAssetUUIDToHandle;
extern std::unordered_map<std::string, std::string> g_AssetManagerAssetUUIDToPath;

template<typename T>
int AssetManager::getOrLoadAssetFromUUID(const std::string &assetUUID) {
    auto it = g_AssetManagerAssetUUIDToHandle.find(assetUUID);

    if (it != g_AssetManagerAssetUUIDToHandle.end()) {
        return it->second;
    }

    auto it2 = g_AssetManagerAssetUUIDToPath.find(assetUUID);

    if (it2 == g_AssetManagerAssetUUIDToPath.end()) {
        std::cout << "could not find asset with uuid: " << assetUUID << std::endl;
        return -1;
    }

    std::string &assetPath = it2->second;

    auto &asset = AssetData<T>::assets.emplace_back(assetPath);

    int assetHandle = static_cast<int>(AssetData<T>::assets.size() - 1);
    g_AssetManagerAssetPathToHandle[assetPath] = assetHandle;

    g_AssetManagerAssetUUIDToHandle[asset.assetUUID()] = assetHandle;

    return assetHandle;
}

template<typename T>
int AssetManager::getOrLoadAssetFromPath(const std::string &assetPath) {
    auto it = g_AssetManagerAssetPathToHandle.find(assetPath);

    if (it != g_AssetManagerAssetPathToHandle.end()) {
        return it->second;
    }

    auto &asset = AssetData<T>::assets.emplace_back(assetPath);

    int assetHandle = static_cast<int>(AssetData<T>::assets.size() - 1);
    g_AssetManagerAssetPathToHandle[assetPath] = assetHandle;

    g_AssetManagerAssetUUIDToHandle[asset.assetUUID()] = assetHandle;

    return assetHandle;
}

template<typename T>
T &AssetManager::getAsset(int assetHandle) {
    return AssetData<T>::assets[assetHandle];
}

template<typename T, typename... Args>
int AssetManager::createAsset(Args &&... args) {
    auto &assetRef = AssetData<T>::assets.emplace_back(std::forward<Args>(args)...);

    int assetHandle = static_cast<int>(AssetData<T>::assets.size() - 1);

    g_AssetManagerAssetUUIDToHandle[assetRef.assetUUID()] = assetHandle;

    return assetHandle;
}

template<typename T>
size_t AssetManager::numAssets() {
    return AssetData<T>::assets.size();
}
