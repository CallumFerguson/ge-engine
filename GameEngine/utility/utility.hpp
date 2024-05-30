#pragma once

#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>

wgpu::Buffer createWebGPUBuffer(wgpu::Device &device, void *data, uint64_t byteLength, wgpu::BufferUsage usage);

void setWindowIcon(GLFWwindow *window, const char *iconPath);
