#include "utility.hpp"

#include "stb_image.h"

namespace GameEngine {

wgpu::Buffer createMappedWebGPUBuffer(wgpu::Device &device, uint64_t byteLength, wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = byteLength;
    bufferDescriptor.usage = usage;
    bufferDescriptor.mappedAtCreation = true;
    return device.CreateBuffer(&bufferDescriptor);
}

wgpu::Buffer createWebGPUBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage) {
    auto buffer = createMappedWebGPUBuffer(device, byteLength, usage);

    auto mappedRange = buffer.GetMappedRange();
    std::memcpy(mappedRange, data, byteLength);
    buffer.Unmap();

    return buffer;
}

void setWindowIcon(GLFWwindow *window, const char *iconPath) {
    // Load the icon image
    int width, height, channels;
    unsigned char *pixels = stbi_load(iconPath, &width, &height, &channels, 4); // Force 4 channels (RGBA)
    if (!pixels) {
        fprintf(stderr, "Failed to load icon image: %s\n", iconPath);
        return;
    }

    // Create GLFW image struct
    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = pixels;

    // Set the window icon
    glfwSetWindowIcon(window, 1, &icon);

    // Free the image memory
    stbi_image_free(pixels);
}

void printMatrix(const glm::mat4 &matrix) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            std::cout << matrix[x][y];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

}