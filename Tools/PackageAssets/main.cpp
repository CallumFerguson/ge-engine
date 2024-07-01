#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"

static const std::unordered_set<std::string> s_packableAssetExtensions = {".png", ".jpg", ".jpeg", ".hdr", ".gltf", ".glb"};

static std::unordered_set<std::string> s_assetsToPackage;
static std::unordered_set<std::string> s_assetsToCopy;

uint64_t hashFile(const std::filesystem::path &path) {
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

    return (static_cast<uint64_t>(s_stringHasher(data)) << 32) | static_cast<uint64_t>(s_stringHasher(path.string()));
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
            s_assetsToPackage.insert(gltfPath.string());
            return;
        }

        for (const auto &entry: std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                bool inUnPackedDir = entry.path().parent_path().filename() == "unpacked";

                if (s_packableAssetExtensions.contains(entry.path().extension().string()) && !inUnPackedDir) {
                    s_assetsToPackage.insert(entry.path().string());
                } else {
                    s_assetsToCopy.insert(entry.path().string());
                }
            } else if (std::filesystem::is_directory(entry.path())) {
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

    std::filesystem::path assetDirPath(std::filesystem::absolute(argv[1]));
    std::filesystem::path outputFilePath(std::filesystem::absolute(argv[2]));

    std::filesystem::path cachePath(std::filesystem::absolute("../../cache"));
    std::filesystem::create_directories(cachePath);

    std::filesystem::path cacheIndexPath = cachePath / "index.json";

    std::ofstream cacheIndexFile(cacheIndexPath);

    findAssetsInDir(assetDirPath);

    std::cout << "assets to package:" << std::endl;
    for (auto &item: s_assetsToPackage) {
        std::cout << item << std::endl;
        std::cout << hashFile(item) << std::endl;
    }

    std::cout << "\nassets to copy:" << std::endl;
    for (auto &item: s_assetsToCopy) {
        std::cout << item << std::endl;
    }
}
