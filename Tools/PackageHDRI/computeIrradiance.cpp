#include "computeIrradiance.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <stb_image_write.h>
#include <numbers>
#include "GameEngine.hpp"

void computeIrradiance(GameEngine::Texture& equirectangularTexture, GameEngine::StreamWriter &streamWriter, const std::string &uuid) {
    GameEngine::TimingHelper time("Compute Irradiance");

    auto &device = GameEngine::WebGPURenderer::device();

    uint32_t textureWidth = 256;
    auto textureHeight = static_cast<uint32_t>(std::round(static_cast<float>(textureWidth) / static_cast<float>(equirectangularTexture.size().width) * static_cast<float>(equirectangularTexture.size().height)));

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {textureWidth, textureHeight, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA32Float;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    auto renderTexture = device.CreateTexture(&textureDescriptor);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = textureDescriptor.size.width * textureDescriptor.size.height * 16;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    wgpu::BufferDescriptor renderInfoBufferDescriptor;
    renderInfoBufferDescriptor.size = 20;
    renderInfoBufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    auto renderInfoBuffer = device.CreateBuffer(&renderInfoBufferDescriptor);

    float sampleDelta = 0.001;
    float phiRange = 2.0 * std::numbers::pi_v<float>;
    float thetaRange = 0.5 * std::numbers::pi_v<float>;
    auto numPhiSamples = static_cast<int32_t>(phiRange / sampleDelta);
    auto numThetaSamples = static_cast<int32_t>(thetaRange / sampleDelta);

//    uint64_t maxSamplesPerDraw = 3750000000;
//    uint32_t renderTextureNumPixels = textureWidth * textureHeight;
//    auto maxPhiSamplePerDraw = static_cast<int32_t>(maxSamplesPerDraw / static_cast<uint64_t>(numThetaSamples * renderTextureNumPixels));

    // having many draw calls seems to be just as fast and doesn't completely block the gpu causing the OS to slow down
    int maxPhiSamplePerDraw = 1;

    std::array<uint8_t, 20> renderInfoData;

    auto updateRenderInfoBuffer = [&]() {
        std::memcpy(renderInfoData.data() + 0, reinterpret_cast<uint8_t *>(&sampleDelta), 4);
        std::memcpy(renderInfoData.data() + 4, reinterpret_cast<uint8_t *>(&phiRange), 4);
        std::memcpy(renderInfoData.data() + 8, reinterpret_cast<uint8_t *>(&thetaRange), 4);
        std::memcpy(renderInfoData.data() + 12, reinterpret_cast<uint8_t *>(&numPhiSamples), 4);
        std::memcpy(renderInfoData.data() + 16, reinterpret_cast<uint8_t *>(&numThetaSamples), 4);
        device.GetQueue().WriteBuffer(renderInfoBuffer, 0, renderInfoData.data(), renderInfoData.size());
    };

    updateRenderInfoBuffer();

    const auto shaderUUID = CALCULATE_IRRADIANCE_SHADER_UUID;
    if (!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            auto &device = GameEngine::WebGPURenderer::device();

            wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

            pipelineDescriptor.layout = nullptr; // auto layout

            wgpu::BlendState blendState;
            blendState.color.operation = wgpu::BlendOperation::Add;
            blendState.color.srcFactor = wgpu::BlendFactor::One;
            blendState.color.dstFactor = wgpu::BlendFactor::One;

            blendState.alpha.operation = wgpu::BlendOperation::Add;
            blendState.alpha.srcFactor = wgpu::BlendFactor::One;
            blendState.alpha.dstFactor = wgpu::BlendFactor::One;

            wgpu::ColorTargetState colorTargetState = {};
            colorTargetState.format = wgpu::TextureFormat::RGBA32Float;
            colorTargetState.blend = &blendState;

            wgpu::FragmentState fragment = {};
            fragment.module = shaderModule;
            fragment.entryPoint = "frag";
            fragment.targetCount = 1;
            fragment.targets = &colorTargetState;

            wgpu::VertexState vertex = {};
            vertex.module = shaderModule;
            vertex.entryPoint = "vert";
            vertex.bufferCount = 0;

            pipelineDescriptor.vertex = vertex;
            pipelineDescriptor.fragment = &fragment;

            pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

            return device.CreateRenderPipeline(&pipelineDescriptor);
        });
    }
    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = renderTexture.CreateView();
    colorAttachment.loadOp = wgpu::LoadOp::Load;
    colorAttachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    std::array<wgpu::BindGroupEntry, 3> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = GameEngine::WebGPURenderer::basicSampler();

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = equirectangularTexture.cachedTextureView();

    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].buffer = renderInfoBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();
    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

    int numRenderPasses = static_cast<int>(std::ceil(static_cast<double>(numPhiSamples) / static_cast<double>(maxPhiSamplePerDraw)));
    std::vector<std::string> completeMessages(numRenderPasses);

    std::cout << "computing irradiance..." << std::endl;

    for (int32_t i = 0; i < numPhiSamples; i += maxPhiSamplePerDraw) {
        int renderPassIndex = i / maxPhiSamplePerDraw;

        auto encoder = device.CreateCommandEncoder();

        auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

        renderPassEncoder.SetPipeline(shader.renderPipeline(false));
        renderPassEncoder.SetBindGroup(0, bindGroup);

        renderPassEncoder.Draw(3, std::min(numPhiSamples - i, maxPhiSamplePerDraw), 0, i);

        renderPassEncoder.End();

        auto commandBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &commandBuffer);

        float percentageFloat = static_cast<float>(renderPassIndex + 1) / static_cast<float>(numRenderPasses);
        int percentageInt = static_cast<int>(percentageFloat * 100.0f);
        if(percentageInt == 0) {
            percentageInt = 1;
        }
        completeMessages[renderPassIndex] = std::to_string(percentageInt) + "% irradiance. pass (" + std::to_string(renderPassIndex + 1) + "/" + std::to_string(numRenderPasses) + ")";
        device.GetQueue().OnSubmittedWorkDone([](WGPUQueueWorkDoneStatus status, void * userdata) {
            std::cout << *reinterpret_cast<std::string*>(userdata) << std::endl;
        }, &completeMessages[renderPassIndex]);

        device.Tick();
    }

    {
        auto encoder = device.CreateCommandEncoder();

        wgpu::ImageCopyTexture src = {};
        src.texture = renderTexture;

        uint32_t bytesPerRow = textureDescriptor.size.width * 16;

        wgpu::ImageCopyBuffer dst = {};
        dst.buffer = readBackBuffer;
        dst.layout.offset = 0;
        dst.layout.bytesPerRow = bytesPerRow;
        dst.layout.rowsPerImage = textureDescriptor.size.height;

        encoder.CopyTextureToBuffer(&src, &dst, &textureDescriptor.size);

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        readBackBuffer.MapAsync(wgpu::MapMode::Read, 0, descriptor.size, [](WGPUBufferMapAsyncStatus status, void *userData) {
            if (status != WGPUBufferMapAsyncStatus_Success) {
                std::cout << "Failed to map buffer for reading." << std::endl;
                exit(1);
            }
        }, nullptr);

        while (readBackBuffer.GetMapState() != wgpu::BufferMapState::Mapped) {
            device.Tick();
        }

        auto *imageFloatsWithAlpha = reinterpret_cast<const float *>(readBackBuffer.GetConstMappedRange());
        if (!imageFloatsWithAlpha) {
            std::cout << "no mapped range data!" << std::endl;
            exit(0);
        }

        std::vector<float> imageFloats(textureDescriptor.size.width * textureDescriptor.size.height * 3);
        for (size_t i = 0; i < textureDescriptor.size.width * textureDescriptor.size.height; i++) {
            imageFloats[i * 3 + 0] = imageFloatsWithAlpha[i * 4 + 0];
            imageFloats[i * 3 + 1] = imageFloatsWithAlpha[i * 4 + 1];
            imageFloats[i * 3 + 2] = imageFloatsWithAlpha[i * 4 + 2];
        }

        streamWriter.writeUUID(uuid);

        uint32_t assetVersion = 0;
        streamWriter.writeRaw(assetVersion);

        std::string imageType = "hdr";
        streamWriter.writeString(imageType);

        uint32_t mipLevelsInFile = 1;
        streamWriter.writeRaw(mipLevelsInFile);

        GameEngine::clearImageDataBuffer();

        stbi_write_hdr_to_func(GameEngine::writeImageDataToBuffer, nullptr, static_cast<int>(textureDescriptor.size.width), static_cast<int>(textureDescriptor.size.height), 3, imageFloats.data());

        uint32_t imageNumBytes = GameEngine::imageDataBuffer().size();
        streamWriter.writeRaw(imageNumBytes);

        streamWriter.writeData(GameEngine::imageDataBuffer().data(), GameEngine::imageDataBuffer().size());

        readBackBuffer.Unmap();
    }
}
