#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"
#include "PackageGLTF/packageGLTF.hpp"
#include "PackageHDRI/packageHDRI.hpp"

// TODO: package images
//static const std::unordered_set<std::string> s_packableAssetExtensions = {".png", ".jpg", ".jpeg", ".hdr", ".gltf", ".glb"};
static const std::unordered_set<std::string> s_packableAssetExtensions = {".hdr", ".gltf", ".glb"};

static std::unordered_set<std::string> s_assetsToPackage;
static std::unordered_set<std::string> s_dirsToCopy;
static std::unordered_set<std::string> s_assetsToCopy;

void packageAsset(const std::filesystem::path &assetPath, const std::filesystem::path &newAssetDir) {
    auto extension = assetPath.extension();
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
        // TODO: package images
    } else if (extension == ".hdr") {
        GameEngineTools::packageHDRI(assetPath, newAssetDir);
    } else if (extension == ".gltf" || extension == ".glb") {
        GameEngineTools::packageGLTF(assetPath, newAssetDir);
    } else {
        std::cout << "packageAsset: unknown extension " << extension << std::endl;
    }
}

uint64_t hashFileContents(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cout << "hashFile: Could not open file." << std::endl;
        exit(1);
    }

    int64_t fileByteLength = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string data(fileByteLength, '\0');

    file.read(data.data(), fileByteLength);

    unsigned long hash = 5381;
    for (size_t i = 0; i < data.size(); i++) {
        hash = ((hash << 5) + hash) + data[i];
    }

//    return (static_cast<uint64_t>(s_stringHasher(data)) << 32) | static_cast<uint64_t>(s_stringHasher(path.string()));
    return static_cast<uint64_t>(hash);
}

uint64_t hashDirectoryContents(const std::filesystem::path &path) {
    uint64_t hash = 5381;
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

void copyEngineAssets(const std::filesystem::path &path, const std::filesystem::path &cachePath) {
    try {
        std::filesystem::create_directories(cachePath / "assets" / "engineAssets");
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                auto dstPath = cachePath / "assets" / "engineAssets" / entry.path().filename();
                if (!std::filesystem::exists(dstPath)) {
                    std::filesystem::copy(entry.path(), dstPath, std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "copyEngineAssets Error: " << e.what() << std::endl;
        exit(1);
    } catch (const std::exception &e) {
        std::cerr << "copyEngineAssets General exception: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <assets_dir> <cache_dir> <engine_assets_dir> <(optional)build_output_dir>" << std::endl;
        return 1;
    }

    std::filesystem::path assetDirPath(argv[1]);
    std::filesystem::path cachePath(argv[2]);
    std::filesystem::path engineAssetsDirPath(argv[3]);
    std::filesystem::path buildOutputDirPath;
    if (argc == 5) {
        buildOutputDirPath = argv[4];
    }

    GameEngine::TimingHelper time("Package Assets");

    // TODO: make Float32Filterable optional
    GameEngine::WebGPURenderer::init(nullptr, {wgpu::FeatureName::Float32Filterable});
    GameEngine::AssetManager::registerAssetUUIDs(engineAssetsDirPath.string());

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

    copyEngineAssets(engineAssetsDirPath, cachePath);

    findAssetsInDir(assetDirPath);

    nlohmann::json inputCacheIndexJSON;
    std::ifstream inputCacheIndexFile(cacheIndexPath, std::ios::binary);
    inputCacheIndexFile >> inputCacheIndexJSON;

    for (nlohmann::json &assetJSON: inputCacheIndexJSON["packedAssets"]) {
        std::string assetPath = assetJSON["assetPath"];
        std::string cachedAssetDir = assetJSON["cachedAssetDir"];
        if (!s_assetsToPackage.contains(assetPath)) {
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
        if (std::filesystem::path(relativeAssetPath).extension() == ".gltf") {
            relativeAssetPath = relativeAssetPath.parent_path();
        }
        auto newAssetDir = (cachedAssetDirPath / relativeAssetPath).replace_extension("");
        std::filesystem::create_directories(newAssetDir);

        auto fileHash = hashFileContents(assetPath);
        auto newFileHash = hashDirectoryContents(newAssetDir);

        nlohmann::json assetJSON;
        assetJSON["hash"] = fileHash;
        assetJSON["cachedHash"] = newFileHash;
        assetJSON["assetPath"] = std::filesystem::absolute(assetPath);
        assetJSON["cachedAssetDir"] = std::filesystem::absolute(newAssetDir);

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
            outputCacheIndexJSON["packedAssets"].push_back(assetJSON);
            continue;
        }

        packageAsset(assetPath, newAssetDir);

        newFileHash = hashDirectoryContents(newAssetDir);
        assetJSON["cachedHash"] = newFileHash;
        outputCacheIndexJSON["packedAssets"].push_back(assetJSON);
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

    if (argc == 5) {
        auto outputAssetsPath = buildOutputDirPath / "assets";
        std::filesystem::remove_all(outputAssetsPath);
        std::filesystem::create_directories(outputAssetsPath);
        for (auto &entry: std::filesystem::recursive_directory_iterator(cachedAssetDirPath)) {
            auto relativeAssetPath = std::filesystem::relative(entry.path(), cachedAssetDirPath);
            auto newAssetPath = outputAssetsPath / relativeAssetPath;

            if (std::filesystem::is_directory(entry.path())) {
                std::filesystem::create_directory(newAssetPath);
            } else if (std::filesystem::is_regular_file(entry.path())) {
                std::filesystem::copy_file(entry.path(), newAssetPath, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
}
