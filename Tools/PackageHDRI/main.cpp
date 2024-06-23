#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <half.hpp>
#include "GameEngine.hpp"

using half_float::half;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    GameEngine::WebGPURenderer::init(nullptr);
    GameEngine::AssetManager::registerAssetUUIDs("../../Sandbox/assets");

    std::filesystem::path inputFilePath(argv[1]);
    std::filesystem::path outputFilePath(argv[2]);
    std::filesystem::create_directories(outputFilePath);
    outputFilePath /= inputFilePath.stem().string() + "_irradiance.hdr";

    auto &device = GameEngine::WebGPURenderer::device();

    int equirectangularTextureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>(inputFilePath.string());
    auto &equirectangularTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(equirectangularTextureHandle);

    while (!equirectangularTexture.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        GameEngine::Texture::writeTextures();
    }

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = equirectangularTexture.size();
    textureDescriptor.format = wgpu::TextureFormat::RGBA16Float;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    auto renderTexture = device.CreateTexture(&textureDescriptor);

    wgpu::BufferDescriptor descriptor;
    auto &size = equirectangularTexture.size();
    descriptor.size = size.width * size.height * 8;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    const auto shaderUUID = CALCULATE_IRRADIANCE_SHADER_UUID;
    if (!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            return GameEngine::WebGPURenderer::createBasicPipeline(shaderModule, false, false, wgpu::TextureFormat::RGBA16Float);
        });
    }
    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = renderTexture.CreateView();
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = GameEngine::WebGPURenderer::basicSampler();

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = equirectangularTexture.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();
    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

    {
        auto encoder = device.CreateCommandEncoder();

        auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

        renderPassEncoder.SetPipeline(shader.renderPipeline(false));
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();

        auto commandBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &commandBuffer);
    }

    {
        auto encoder = device.CreateCommandEncoder();

        wgpu::ImageCopyTexture src = {};
        src.texture = renderTexture;

        uint32_t bytesPerRow = equirectangularTexture.size().width * 8;
        if (bytesPerRow < 256) {
            std::cout << "Bytes per row cannot be less than 256" << std::endl;
            return 1;
        }

        wgpu::ImageCopyBuffer dst = {};
        dst.buffer = readBackBuffer;
        dst.layout.offset = 0;
        dst.layout.bytesPerRow = bytesPerRow;
        dst.layout.rowsPerImage = equirectangularTexture.size().height;

        encoder.CopyTextureToBuffer(&src, &dst, &equirectangularTexture.size());

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        readBackBuffer.MapAsync(wgpu::MapMode::Read, 0, descriptor.size, [](WGPUBufferMapAsyncStatus status, void *userData) {
            if (status != WGPUBufferMapAsyncStatus_Success) {
                std::cout << "Failed to map buffer for reading." << std::endl;
            }
        }, nullptr);

        while (readBackBuffer.GetMapState() != wgpu::BufferMapState::Mapped) {
            device.Tick();
        }

        auto *imageHalfs = reinterpret_cast<const half *>(readBackBuffer.GetConstMappedRange());
        if (!imageHalfs) {
            std::cout << "no mapped range data!" << std::endl;
            return 1;
        }

        std::vector<float> imageFloats(size.width * size.height * 3);
        for (size_t i = 0; i < size.width * size.height; i++) {
            imageFloats[i * 3 + 0] = imageHalfs[i * 4 + 0];
            imageFloats[i * 3 + 1] = imageHalfs[i * 4 + 1];
            imageFloats[i * 3 + 2] = imageHalfs[i * 4 + 2];
        }

        stbi_write_hdr(outputFilePath.string().c_str(), static_cast<int>(size.width), static_cast<int>(size.height), 3, imageFloats.data());

        readBackBuffer.Unmap();
    }
}
