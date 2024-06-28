#include "AssetManager.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "../Utility/utility.hpp"

namespace GameEngine {

std::unordered_map<std::string, int> g_AssetManagerAssetPathToHandle;
std::unordered_map<std::string, int> g_AssetManagerAssetUUIDToHandle;
std::unordered_map<std::string, std::string> g_AssetManagerAssetUUIDToPath;

void AssetManager::registerAssetUUIDs(const std::string &assetsPath) {
    std::vector<std::string> extensions = {".gemesh", ".getexture", ".wgsl", ".gematerial", ".geenvironmentmap"};

    for (const auto &entry: std::filesystem::recursive_directory_iterator(assetsPath)) {
        for (const auto &extension: extensions) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                std::ifstream inputFile(entry.path(), std::ios::binary);
                if (!inputFile) {
                    std::cerr << "Error: assetUUIDToPath could not open file " << entry.path() << " for reading!" << std::endl;
                    break;
                }

                if (extension == ".wgsl") {
                    inputFile.seekg(2, std::ios::cur);
                }

                char uuid[37];
                uuid[36] = '\0';
                inputFile.read(uuid, 36);

                if (isUUID(uuid)) {
                    g_AssetManagerAssetUUIDToPath[uuid] = entry.path().string();
                } else {
                    if (extension == ".wgsl") {
                        std::cout << "shader missing uuid. first line of file should be \"//aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa\"" << std::endl;
                    }
                    std::cout << "could not read uuid in asset: " << entry.path() << std::endl;
                }
            }
        }
    }
}

}
