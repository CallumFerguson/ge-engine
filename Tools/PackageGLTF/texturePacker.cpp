#include "texturePacker.hpp"

#include <iostream>
#include <stb_image_write.h>
#include "GameEngine.hpp"

namespace GameEngineTools {

void writeImageDataToFile(void *context, void *data, int size) {
    static_cast<std::ofstream *>(context)->write(reinterpret_cast<const char *>(data), size);
}

void writeGLTFTextureImageFile(const tinygltf::Image &image, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID) {
    std::ofstream outputFile(outputFilePath / (name + ".getexture"), std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    outputFile << textureUUID;

    stbi_write_jpg_to_func(writeImageDataToFile, &outputFile, image.width, image.height, image.component, image.image.data(), 90);

    // PNG ends up with files that are larger than the gltf source pngs and paint.net pngs, so maybe these should be copied over directly if possible
//    stbi_write_png_to_func(writeImageDataToFile, &outputFile, image.width, image.height, image.component, image.image.data(), image.width * image.component);
}

}
