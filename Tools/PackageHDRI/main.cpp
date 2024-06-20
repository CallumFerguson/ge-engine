#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <half.hpp>
#include "GameEngine.hpp"

using half_float::half;

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
    float *floatImage = stbi_loadf(inputFilePath.string().c_str(), &width, &height, &channelsInFile, 4);
    if (!floatImage) {
        std::cout << "could not load image" << std::endl;
        return 1;
    }

    // float image is twice the size needed for the image as halfs, so just reuse the memory allocated by stb
    for (int i = 0; i < width * height * 4; i++) {
        reinterpret_cast<half *>(floatImage)[i] = floatImage[i];
    }

    std::cout << reinterpret_cast<half *>(floatImage)[0] << std::endl;

    stbi_image_free(floatImage);
}
