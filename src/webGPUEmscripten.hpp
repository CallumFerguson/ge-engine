#pragma once

#include <webgpu/webgpu_cpp.h>

void resizeCanvas(wgpu::Device &device);

uint32_t getCanvasWidth();

uint32_t getCanvasHeight();

void updateCursor();
