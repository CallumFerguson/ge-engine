#include "emscriptenUtility.hpp"

#include "imgui.h"
#include <emscripten.h>

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

}
