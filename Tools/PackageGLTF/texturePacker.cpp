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

    if (image.component != 4) {
        // TODO: this might not even be needed. if it is, there should be some way to make it not needed
        std::cout << "writeGLTFTextureImageFile image.component should be 4" << std::endl;
    }

    auto &device = GameEngine::WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1};
    textureDescriptor.mipLevelCount = GameEngine::numMipLevels(textureDescriptor.size);
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    auto texture = device.CreateTexture(&textureDescriptor);

    wgpu::ImageCopyTexture destination;
    destination.texture = std::move(texture);

    wgpu::TextureDataLayout dataLayout;
    dataLayout.bytesPerRow = image.width * image.component;
    dataLayout.rowsPerImage = image.height;

    wgpu::Extent3D size = {static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1};

    device.GetQueue().WriteTexture(&destination, image.image.data(), image.width * image.height * image.component, &dataLayout, &size);

    GameEngine::generateMipmap(device, destination.texture);
}

}
