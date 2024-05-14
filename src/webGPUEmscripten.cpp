#include "webGPUEmscripten.hpp"

#include <emscripten.h>

void resizeCanvas(wgpu::Device &device) {
    wgpu::SupportedLimits supportedLimits = {};
    device.GetLimits(&supportedLimits);

    // @formatter:off
    EM_ASM({
                   const canvas = Module.canvas;
                   const width = Math.max(1, Math.min($0, canvas.clientWidth));
                   const height = Math.max(1, Math.min($0, canvas.clientHeight));

                   const needResize = width !== canvas.width || height !== canvas.height;
                   if (needResize) {
                       canvas.width = width;
                       canvas.height = height;
                   }
           }, supportedLimits.limits.maxTextureDimension2D);
    // @formatter:on
}

// @formatter:off
uint32_t getCanvasWidth() {
    return EM_ASM_INT(
    if (Module.canvas) {
        return canvas.width;
    }
    return 0;
    );
}

uint32_t getCanvasHeight() {
    return EM_ASM_INT(
    if (Module.canvas) {
        return canvas.height;
    }
    return 0;
    );
}
// @formatter:on
