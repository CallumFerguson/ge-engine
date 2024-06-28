#include "textureUtils.hpp"

namespace GameEngine {

// from dawn/src/dawn/utils/TextureUtils.cpp
uint32_t getTexelBlockSizeInBytes(wgpu::TextureFormat textureFormat) {
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::Stencil8:
            return 1u;

        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
            return 2u;

        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
            return 4u;

        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
            return 8u;

        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
            return 16u;

        case wgpu::TextureFormat::Depth16Unorm:
            return 2u;

        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
            return 4u;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
            return 8u;

        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return 16u;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
            return 8u;

        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:
            return 16u;

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            return 16u;

        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:

            // Block size of a multi-planar format depends on aspect.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:

        case wgpu::TextureFormat::Undefined:
            break;
    }

    std::cout << "GetTexelBlockSizeInBytes texture does not have a size" << std::endl;
    return 0;
}

}
