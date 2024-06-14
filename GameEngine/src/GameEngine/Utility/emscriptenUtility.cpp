#include "emscriptenUtility.hpp"

#include <imgui.h>
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

namespace GameEngine {

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

// @formatter:off
EM_JS(void, copyExternalImageToTexture, (int deviceJsHandle, int textureJsHandle, const uint8_t *data, int size), {
    (async () => {
        const device = JsValStore.get(deviceJsHandle);
        const texture = JsValStore.get(textureJsHandle);

        const blob = new Blob([HEAPU8.subarray(data, data + size)], {
            type:
            'image/jpeg'
        });

        const imageBitmap = await createImageBitmap(blob, {colorSpaceConversion: 'none'});

        device.queue.copyExternalImageToTexture(
            {source: imageBitmap, flipY: true},
            {texture},
            {width: imageBitmap.width, height: imageBitmap.height}
        );

        JsValStore.remove(deviceJsHandle);
        JsValStore.remove(textureJsHandle);
    })();
});
// @formatter:on

void writeTextureJSAsync(const wgpu::Device &device, const wgpu::Texture &texture, const std::vector<uint8_t> &data) {
    int deviceJsHandle = emscripten_webgpu_export_device(device.Get());
    int textureJsHandle = emscripten_webgpu_export_texture(texture.Get());
    copyExternalImageToTexture(deviceJsHandle, textureJsHandle, data.data(), static_cast<int>(data.size()));
}

}
