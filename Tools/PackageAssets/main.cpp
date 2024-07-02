#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"

static const std::unordered_set<std::string> s_packableAssetExtensions = {".png", ".jpg", ".jpeg", ".hdr", ".gltf", ".glb"};

static std::unordered_set<std::string> s_assetsToPackage;
static std::unordered_set<std::string> s_dirsToCopy;
static std::unordered_set<std::string> s_assetsToCopy;

void packageAsset(const std::filesystem::path &assetPath, const std::filesystem::path &newAssetDir) {
    std::cout << "packageAsset:" << std::endl;
    std::cout << assetPath << std::endl;
}

uint64_t hashFileContents(const std::filesystem::path &path) {
    static std::hash<std::string> s_stringHasher;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cout << "hashFile: Could not open file." << std::endl;
        exit(1);
    }

    int64_t fileByteLength = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string data(fileByteLength, '\0');

    file.read(data.data(), fileByteLength);

//    return (static_cast<uint64_t>(s_stringHasher(data)) << 32) | static_cast<uint64_t>(s_stringHasher(path.string()));
    return static_cast<uint64_t>(s_stringHasher(data));
}

uint64_t hashDirectoryContents(const std::filesystem::path &path) {
    static std::hash<std::string> s_stringHasher;

    uint64_t hash;
    for (const auto &entry: std::filesystem::directory_iterator(path)) {
        if (std::filesystem::is_regular_file(entry.status())) {
            hash ^= hashFileContents(entry.path()) + 0x9e3779b97f4a7c15 + (hash << 6) + (hash >> 2);
        }
    }

    return hash;
}

std::filesystem::path getDirGLTFPath(const std::filesystem::path &dirPath) {
    try {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "dirContainsGLTF: The provided path is not a directory or does not exist." << std::endl;
            return {};
        }

        for (const auto &entry: std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                if (entry.path().extension() == ".gltf") {
                    return entry.path();
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    }

    return {};
}

void findAssetsInDir(const std::filesystem::path &dirPath) {
    try {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "findAssetsInDir: The provided path is not a directory or does not exist." << std::endl;
            exit(1);
        }

        auto gltfPath = getDirGLTFPath(dirPath);
        if (!gltfPath.empty()) {
            s_assetsToPackage.insert(std::filesystem::absolute(gltfPath).string());
            return;
        }

        for (const auto &entry: std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                bool inUnPackedDir = entry.path().parent_path().filename() == "unpacked";

                if (s_packableAssetExtensions.contains(entry.path().extension().string()) && !inUnPackedDir) {
                    s_assetsToPackage.insert(std::filesystem::absolute(entry.path()).string());
                } else {
                    s_assetsToCopy.insert(std::filesystem::absolute(entry.path()).string());
                }
            } else if (std::filesystem::is_directory(entry.path())) {
                s_dirsToCopy.insert(std::filesystem::absolute(entry.path()).string());
                findAssetsInDir(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit(1);
    } catch (const std::exception &e) {
        std::cerr << "General exception: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    GameEngine::TimingHelper time("total");

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    std::filesystem::path assetDirPath(argv[1]);

    std::filesystem::path cachePath("../../cache");
    std::filesystem::create_directories(cachePath);

    std::filesystem::path cachedAssetDirPath = cachePath / "assets";
    std::filesystem::create_directories(cachedAssetDirPath);

    std::filesystem::path cacheIndexPath = cachePath / "index.json";

    if (!std::filesystem::exists(cacheIndexPath)) {
        nlohmann::json cacheIndexJSON;
        cacheIndexJSON["packedAssets"] = nlohmann::json::array();
        cacheIndexJSON["copiedDirs"] = nlohmann::json::array();
        cacheIndexJSON["copiedAssets"] = nlohmann::json::array();

        std::ofstream cacheIndexFile(cacheIndexPath);
        cacheIndexFile << cacheIndexJSON;
    }

    findAssetsInDir(assetDirPath);

    nlohmann::json inputCacheIndexJSON;
    std::ifstream inputCacheIndexFile(cacheIndexPath, std::ios::binary);
    inputCacheIndexFile >> inputCacheIndexJSON;

    for (nlohmann::json &assetJSON: inputCacheIndexJSON["packedAssets"]) {
        std::string assetPath = assetJSON["assetPath"];
        std::string cachedAssetDir = assetJSON["cachedAssetDir"];
        if (!s_assetsToPackage.contains(assetPath)) {
            std::cout << "remove asset" << std::endl;
            
            std::filesystem::remove_all(cachedAssetDir);
        }
    }

    for (nlohmann::json &assetJSON: inputCacheIndexJSON["copiedAssets"]) {
        std::string assetPath = assetJSON["assetPath"];
        std::string cachedAssetPath = assetJSON["cachedAssetPath"];
        if (!s_assetsToCopy.contains(assetPath)) {
            std::filesystem::remove(cachedAssetPath);
        }
    }

    for (nlohmann::json &dirJSON: inputCacheIndexJSON["copiedDirs"]) {
        std::string dirPath = dirJSON["dirPath"];
        std::string cachedDirPath = dirJSON["cachedDirPath"];
        if (!s_dirsToCopy.contains(dirPath)) {
            std::filesystem::remove(cachedDirPath);

        }
    }

    nlohmann::json outputCacheIndexJSON;
    outputCacheIndexJSON["packedAssets"] = nlohmann::json::array();
    outputCacheIndexJSON["copiedDirs"] = nlohmann::json::array();
    outputCacheIndexJSON["copiedAssets"] = nlohmann::json::array();

    for (auto &assetPath: s_assetsToPackage) {
        auto relativeAssetPath = std::filesystem::relative(assetPath, assetDirPath);
        auto newAssetDir = (cachedAssetDirPath / relativeAssetPath).replace_extension("");
        std::filesystem::create_directories(newAssetDir);

        auto fileHash = hashFileContents(assetPath);
        auto newFileHash = hashDirectoryContents(newAssetDir);

        nlohmann::json assetJSON;
        assetJSON["hash"] = fileHash;
        assetJSON["cachedHash"] = newFileHash;
        assetJSON["assetPath"] = std::filesystem::absolute(assetPath);
        assetJSON["cachedAssetDir"] = std::filesystem::absolute(newAssetDir);

        outputCacheIndexJSON["packedAssets"].push_back(assetJSON);

        bool assetIsPackagedAndUpToDate = false;
        for (auto &packedAssetJSON: inputCacheIndexJSON["packedAssets"]) {
            if (packedAssetJSON["assetPath"] == assetPath) {
                bool assetHashMatches = packedAssetJSON["hash"].get<uint64_t>() == fileHash;
                bool cachedAssetHashMatches = packedAssetJSON["cachedHash"].get<uint64_t>() == newFileHash;
                if (assetHashMatches && cachedAssetHashMatches) {
                    assetIsPackagedAndUpToDate = true;
                }
                break;
            }
        }

        if (assetIsPackagedAndUpToDate) {
            continue;
        }

        packageAsset(assetPath, newAssetDir);
    }

    for (std::filesystem::path dirPath: s_dirsToCopy) {
        auto relativeDirPath = std::filesystem::relative(dirPath, assetDirPath);
        auto newDirPath = cachedAssetDirPath / relativeDirPath;

        nlohmann::json dirJSON;
        dirJSON["dirPath"] = std::filesystem::absolute(dirPath);
        dirJSON["cachedDirPath"] = std::filesystem::absolute(newDirPath);

        outputCacheIndexJSON["copiedDirs"].push_back(dirJSON);

        if (std::filesystem::exists(newDirPath)) {
            continue;
        }

        std::filesystem::create_directories(newDirPath);
    }

    for (std::filesystem::path assetPath: s_assetsToCopy) {
        auto relativeAssetPath = std::filesystem::relative(assetPath, assetDirPath);
        auto newAssetPath = cachedAssetDirPath / relativeAssetPath;

        auto fileHash = hashFileContents(assetPath);

        nlohmann::json assetJSON;
        assetJSON["hash"] = fileHash;
        assetJSON["assetPath"] = std::filesystem::absolute(assetPath);
        assetJSON["cachedAssetPath"] = std::filesystem::absolute(newAssetPath);

        outputCacheIndexJSON["copiedAssets"].push_back(assetJSON);

        if (std::filesystem::exists(newAssetPath)) {
            auto newFileHash = hashFileContents(newAssetPath);
            if (newFileHash == fileHash) {
                // file is the same so skip the copy
                continue;
            }
        }

        std::filesystem::copy_file(assetPath, newAssetPath, std::filesystem::copy_options::overwrite_existing);
    }

    std::ofstream outputCacheIndexFile(cacheIndexPath, std::ios::binary);
    outputCacheIndexFile << outputCacheIndexJSON.dump(2);
}
