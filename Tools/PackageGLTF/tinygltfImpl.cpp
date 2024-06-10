// I don't know why this part is needed. I think it might even be for ImGui
#ifdef APIENTRY
#undef APIENTRY
#endif
// ==================================

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION // don't need because GameEngine stbImpl.cpp has it
//#define STB_IMAGE_WRITE_IMPLEMENTATION // don't need because GameEngine stbImpl.cpp has it

#include <tiny_gltf.h>
