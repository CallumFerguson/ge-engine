#include "Texture.hpp"

#include <vector>
#include <nlohmann/json.hpp>
#include "../Utility/Random.hpp"
#include "stb_image.h"

namespace GameEngine {

Texture::Texture() : Asset(Random::uuid()) {}

Texture::Texture(const std::string &assetPath) {
    std::ifstream assetFile(assetPath, std::ios::binary | std::ios::ate);
    if (!assetFile) {
        std::cerr << "Error: [Texture] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    auto imageByteLength = static_cast<std::streamsize>(assetFile.tellg()) - 36;
    assetFile.seekg(0, std::ios::beg);

    char uuid[37];
    uuid[36] = '\0';
    assetFile.read(uuid, 36);
    m_assetUUID = uuid;

    std::cout << "start reading data" << std::endl;

    std::vector<uint8_t> imageData(imageByteLength);
    assetFile.read(reinterpret_cast<char *>(imageData.data()), imageByteLength);

    std::cout << "start stb image load" << std::endl;

    // Width, height, and number of channels in the image
    int width, height, channels;

    // Load the image from memory
    unsigned char *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, &channels, 0);
    if (image == nullptr) {
        std::cerr << "Failed to load image from memory" << std::endl;
        return;
    }

    std::cout << "Loaded image from memory: " << imageData.size() << std::endl;
    std::cout << "Width: " << width << ", Height: " << height << ", Channels: " << channels << std::endl;

    stbi_image_free(image);

    //TODO: profile this vs shipping data off to js land
}

}
