#include "webGPUEmscripten.hpp"

#include <imgui.h>
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

void updateCursor() {
    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    switch (cursor) {
        case ImGuiMouseCursor_None:
            EM_ASM({ document.body.style.cursor = "none"; });
            break;
        case ImGuiMouseCursor_Arrow:
            EM_ASM({ document.body.style.cursor = "default"; });
            break;
        case ImGuiMouseCursor_TextInput:
            EM_ASM({ document.body.style.cursor = "text"; });
            break;
        case ImGuiMouseCursor_ResizeAll:
            EM_ASM({ document.body.style.cursor = "all-scroll"; });
            break;
        case ImGuiMouseCursor_ResizeNS:
            EM_ASM({ document.body.style.cursor = "ns-resize"; });
            break;
        case ImGuiMouseCursor_ResizeEW:
            EM_ASM({ document.body.style.cursor = "ew-resize"; });
            break;
        case ImGuiMouseCursor_ResizeNESW:
            EM_ASM({ document.body.style.cursor = "nesw-resize"; });
            break;
        case ImGuiMouseCursor_ResizeNWSE:
            EM_ASM({ document.body.style.cursor = "nwse-resize"; });
            break;
        case ImGuiMouseCursor_Hand:
            EM_ASM({ document.body.style.cursor = "move"; });
            break;
        case ImGuiMouseCursor_NotAllowed:
            EM_ASM({ document.body.style.cursor = "not-allowed"; });
            break;
        default:
            EM_ASM({ document.body.style.cursor = "default"; });
            break;
    }
}
