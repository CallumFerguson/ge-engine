#include "computePreFilter.hpp"

#include <stb_image_write.h>

static const int roughnessMipLevels = 5;

void computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &preFilterOutputPath) {
    GameEngine::TimingHelper time("Compute Pre Filter");

    auto &device = GameEngine::WebGPURenderer::device();

    GameEngine::generateMipmap(device, equirectangularTexture.texture());

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = equirectangularTexture.size();
    textureDescriptor.mipLevelCount = roughnessMipLevels;
    textureDescriptor.format = wgpu::TextureFormat::RGBA32Float;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    auto equirectangularTextureOutput = device.CreateTexture(&textureDescriptor);

    const auto shaderUUID = CALCULATE_PRE_FILTER_SHADER_UUID;
    if (!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            return GameEngine::WebGPURenderer::createBasicPipeline(shaderModule, false, false, wgpu::TextureFormat::RGBA32Float);
        });
    }
    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    std::array<wgpu::BindGroupEntry, 3> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = GameEngine::WebGPURenderer::basicSampler();

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = equirectangularTexture.cachedTextureView();

    wgpu::BufferDescriptor roughnessBufferDescriptor;
    roughnessBufferDescriptor.size = 4;
    roughnessBufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    auto roughnessBuffer = device.CreateBuffer(&roughnessBufferDescriptor);

    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].buffer = roughnessBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();
    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

    for (int mipLevel = 0; mipLevel < roughnessMipLevels; mipLevel++) {
        float roughness = static_cast<float>(mipLevel) / (static_cast<float>(roughnessMipLevels) - 1.0f);
        device.GetQueue().WriteBuffer(roughnessBuffer, 0, &roughness, 4);

        wgpu::TextureViewDescriptor outputTextureViewDescriptor;
        outputTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        outputTextureViewDescriptor.baseArrayLayer = 0;
        outputTextureViewDescriptor.arrayLayerCount = 1;
        outputTextureViewDescriptor.baseMipLevel = mipLevel;
        outputTextureViewDescriptor.mipLevelCount = 1;

        colorAttachment.view = equirectangularTextureOutput.CreateView(&outputTextureViewDescriptor);

        auto encoder = device.CreateCommandEncoder();

        auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

        renderPassEncoder.SetPipeline(shader.renderPipeline(false));
        renderPassEncoder.SetBindGroup(0, bindGroup);

        renderPassEncoder.Draw(3);

        renderPassEncoder.End();

        auto commandBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &commandBuffer);
    }

    wgpu::BufferDescriptor descriptor;
    descriptor.size = equirectangularTexture.size().width * equirectangularTexture.size().width * 16;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    for (uint32_t level = 0; level < roughnessMipLevels; level++) {
        uint32_t mipWidth = std::max(1u, equirectangularTexture.size().width >> level);
        uint32_t mipHeight = std::max(1u, equirectangularTexture.size().height >> level);
        uint32_t mipSize = mipWidth * mipHeight * 16;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ImageCopyTexture src = {};
        src.texture = equirectangularTextureOutput;
        src.mipLevel = level;

        uint32_t bytesPerRow = mipWidth * 16;
        uint32_t paddedBytesPerRow = std::max(256u, bytesPerRow);

        wgpu::ImageCopyBuffer dst = {};
        dst.buffer = readBackBuffer;
        dst.layout.offset = 0;
        dst.layout.bytesPerRow = paddedBytesPerRow;
        dst.layout.rowsPerImage = mipHeight;

        wgpu::Extent3D extent = {mipWidth, mipHeight, 1};
        encoder.CopyTextureToBuffer(&src, &dst, &extent);

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        readBackBuffer.MapAsync(wgpu::MapMode::Read, 0, mipSize, [](WGPUBufferMapAsyncStatus status, void *userData) {
            if (status != WGPUBufferMapAsyncStatus_Success) {
                std::cout << "Failed to map buffer for reading." << std::endl;
            }
        }, nullptr);

        while (readBackBuffer.GetMapState() != wgpu::BufferMapState::Mapped) {
            device.Tick();
        }

        auto *data = reinterpret_cast<const uint8_t *>(readBackBuffer.GetConstMappedRange(0, mipSize));
        if (!data) {
            std::cout << "no mapped range data!" << std::endl;
            return;
        }

        const uint8_t *dataWithoutPadding = data;
        std::vector<uint8_t> dataWithoutPaddingVector;

        if (bytesPerRow < paddedBytesPerRow) {
            dataWithoutPaddingVector.resize(bytesPerRow * mipHeight);
            for (uint32_t row = 0; row < mipHeight; row++) {
                for (uint32_t column = 0; column < bytesPerRow; column++) {
                    dataWithoutPaddingVector[row * bytesPerRow + column] = data[row * paddedBytesPerRow + column];
                }
            }
            dataWithoutPadding = dataWithoutPaddingVector.data();
        }

        auto floatData = reinterpret_cast<const float *>(dataWithoutPadding);

        std::vector<float> imageFloats(mipWidth * mipHeight * 3);
        for (size_t i = 0; i < mipWidth * mipHeight; i++) {
            imageFloats[i * 3 + 0] = floatData[i * 4 + 0];
            imageFloats[i * 3 + 1] = floatData[i * 4 + 1];
            imageFloats[i * 3 + 2] = floatData[i * 4 + 2];
        }

        std::filesystem::path filePath = preFilterOutputPath.string() + "_mip_" + std::to_string(level) + ".hdr";
        stbi_write_hdr(filePath.string().c_str(), static_cast<int>(mipWidth), static_cast<int>(mipHeight), 3, imageFloats.data());

        readBackBuffer.Unmap();
    }
}
