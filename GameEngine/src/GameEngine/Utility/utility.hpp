#pragma once

#include <string>
#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace GameEngine {

wgpu::Buffer createMappedWebGPUBuffer(wgpu::Device &device, uint64_t byteLength, wgpu::BufferUsage usage);

wgpu::Buffer createWebGPUBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage);

void setWindowIcon(GLFWwindow *window, const char *iconPath);

void printMatrix(const glm::mat4 &matrix);

bool isUUID(const std::string &uuid);

}
