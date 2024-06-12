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

void resetCanvas() {
    EM_ASM({
               const newCanvas = document.createElement("canvas");
               newCanvas.id = "canvas";

               const oldCanvas = Module.canvas;

               oldCanvas.parentElement.replaceChild(newCanvas, oldCanvas);
               Module.canvas = newCanvas;
           });
}

//static bool s_writeTextureDone = false;

//extern "C" {
//
//void writeTextureCallback() {
//    s_writeTextureDone = true;
//}
//
//}

// @formatter:off
EM_JS(void, writeTexture, (int deviceJsHandle, int textureJsHandle, const uint8_t *data, int size), {
    (async () => {
            const start = performance.now();

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

            const end = performance.now();
            const time = end - start;
            console.log("Execution time: " + time +" milliseconds");

//            Module.ccall('writeTextureCallback', null, [], []);
    })();
});
// @formatter:on

void writeTextureJS(const wgpu::Device &device, const wgpu::Texture &texture, const std::vector<uint8_t> &data) {
    int deviceJsHandle = emscripten_webgpu_export_device(device.Get());
    int textureJsHandle = emscripten_webgpu_export_texture(texture.Get());

//    s_writeTextureDone = false;
    writeTexture(deviceJsHandle, textureJsHandle, data.data(), static_cast<int>(data.size()));

//    while (!s_writeTextureDone) {
//        std::cout << s_writeTextureDone << std::endl;
//        emscripten_sleep(200);
//    }

    emscripten_webgpu_release_js_handle(deviceJsHandle);
    emscripten_webgpu_release_js_handle(textureJsHandle);
}

}
