#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include "GameEngine.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

//    GameEngine::WebGPURenderer::init(nullptr);

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);
    std::filesystem::create_directories(outputFilePath);
    outputFilePath /= inputFilePath.stem().string() + ".png";

//    auto device = GameEngine::WebGPURenderer::device();

    int width, height, channelsInFile;
    float *imageData = stbi_loadf(inputFilePath.string().c_str(), &width, &height, &channelsInFile, 4);
    if (!imageData) {
        std::cout << "could not load image" << std::endl;
        return 1;
    }

    stbi_image_free(imageData);

    unsigned char *ldrData = new unsigned char[width * height * 4];
    for (int i = 0; i < width * height * 4; ++i) {
        // Simple tone mapping and gamma correction
        float mappedValue = imageData[i] / (imageData[i] + 1.0f); // tone mapping
        mappedValue = powf(mappedValue, 1.0f / 2.2f); // gamma correction
        ldrData[i] = static_cast<unsigned char>(mappedValue * 255.0f);
    }

    std::cout << width << std::endl;
    std::cout << height << std::endl;
    std::cout << channelsInFile << std::endl;

    stbi_write_png(outputFilePath.string().c_str(), width, height, 4, ldrData, width * 4);
}
