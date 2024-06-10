// I don't know why this part is needed. I think it might even be for ImGui
#ifdef APIENTRY
#undef APIENTRY
#endif
// ==================================

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
