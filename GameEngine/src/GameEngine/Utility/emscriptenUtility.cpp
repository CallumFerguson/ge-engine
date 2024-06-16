#include "emscriptenUtility.hpp"

#include <imgui.h>
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#include "../Rendering/Backends/WebGPU/generateMipmapWebGPU.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"

namespace GameEngine {

// @formatter:off
EM_JS(void, setCursor, (const char *cursor), {
    if(Module.canvas) {
        Module.canvas.style.cursor = UTF8ToString(cursor);
    }
});
// @formatter:on

void updateCursor() {
    ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    switch (cursor) {
        case ImGuiMouseCursor_None:
            setCursor("none");
            break;
        case ImGuiMouseCursor_Arrow:
            setCursor("default");
            break;
        case ImGuiMouseCursor_TextInput:
            setCursor("text");
            break;
        case ImGuiMouseCursor_ResizeAll:
            setCursor("all-scroll");
            break;
        case ImGuiMouseCursor_ResizeNS:
            setCursor("ns-resize");
            break;
        case ImGuiMouseCursor_ResizeEW:
            setCursor("ew-resize");
            break;
        case ImGuiMouseCursor_ResizeNESW:
            setCursor("nesw-resize");
            break;
        case ImGuiMouseCursor_ResizeNWSE:
            setCursor("nwse-resize");
            break;
        case ImGuiMouseCursor_Hand:
            setCursor("move");
            break;
        case ImGuiMouseCursor_NotAllowed:
            setCursor("not-allowed");
            break;
        default:
            setCursor("default");
            break;
    }
}

// @formatter:off
void resetCanvas() {
    EM_ASM({
       const newCanvas = document.createElement("canvas");
       newCanvas.id = "canvas";

       const oldCanvas = Module.canvas;

       oldCanvas.parentElement.replaceChild(newCanvas, oldCanvas);
       Module.canvas = newCanvas;
   });
}
// @formatter:on

extern "C" {

void copyExternalImageToTextureFinishCallback(int textureJsHandle, bool shouldGenerateMipmap) {
    wgpu::Texture texture = wgpu::Texture::Acquire(emscripten_webgpu_import_texture(textureJsHandle));
    emscripten_webgpu_release_js_handle(textureJsHandle);
    if (shouldGenerateMipmap) {
        generateMipmap(WebGPURenderer::device(), texture);
    }
}

}

// @formatter:off
EM_JS(void, copyExternalImageToTexture, (int deviceJsHandle, int textureJsHandle, const uint8_t *data, int size, bool shouldGenerateMipmap, int mipLevel), {
    (async () => {
        const device = JsValStore.get(deviceJsHandle);
        const texture = JsValStore.get(textureJsHandle);

        const blob = new Blob([HEAPU8.subarray(data, data + size)], {
            type: "image/jpeg"
        });

        const imageBitmap = await createImageBitmap(blob, {colorSpaceConversion: "none"});

        device.queue.copyExternalImageToTexture(
            {source: imageBitmap, flipY: false},
            {texture, mipLevel},
            {width: imageBitmap.width, height: imageBitmap.height}
        );

        JsValStore.remove(deviceJsHandle);
        Module.ccall("copyExternalImageToTextureFinishCallback", null, ["number", "boolean"], [textureJsHandle, shouldGenerateMipmap]);
    })();
});
// @formatter:on

void writeTextureJSAsync(const wgpu::Device &device, const wgpu::Texture &texture, const uint8_t *data, int dataSize, bool shouldGenerateMipmap, int mipLevel) {
    int deviceJsHandle = emscripten_webgpu_export_device(device.Get());
    int textureJsHandle = emscripten_webgpu_export_texture(texture.Get());
    copyExternalImageToTexture(deviceJsHandle, textureJsHandle, data, dataSize, shouldGenerateMipmap, mipLevel);
}

}
