#include "utility.hpp"

#include "stb_image.h"

wgpu::Buffer createWebGPUBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = byteLength;
    bufferDescriptor.usage = usage;
    bufferDescriptor.mappedAtCreation = true;
    auto buffer = device.CreateBuffer(&bufferDescriptor);

    auto mappedRange = buffer.GetMappedRange();
    std::memcpy(mappedRange, data, bufferDescriptor.size);
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
