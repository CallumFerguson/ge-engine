#include "computePreFilter.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

static const uint32_t roughnessMipLevels = 5;

std::ostringstream computePreFilter(GameEngine::Texture &equirectangularTexture, const std::filesystem::path &inputFilePath) {
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
    colorAttachment.loadOp = wgpu::LoadOp::Load;
    colorAttachment.storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    std::array<wgpu::BindGroupEntry, 4> bindGroupEntries;

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

    uint32_t sampleCount = 65536;
    uint32_t samplesPerDraw = 1024;
    uint32_t numDraws = sampleCount / samplesPerDraw;
    uint32_t newSampleCount = samplesPerDraw * numDraws;
    if(newSampleCount != sampleCount) {
        std::cout << "requested sample count: " << sampleCount << ". Actual sample count: " << newSampleCount << std::endl;
        sampleCount = newSampleCount;
    }

    wgpu::BufferDescriptor renderInfoBufferDescriptor;
    renderInfoBufferDescriptor.mappedAtCreation = true;
    renderInfoBufferDescriptor.size = 12;
    renderInfoBufferDescriptor.usage = wgpu::BufferUsage::Uniform;
    auto renderInfoBuffer = device.CreateBuffer(&renderInfoBufferDescriptor);
    auto renderInfoBufferData = static_cast<uint8_t *>(renderInfoBuffer.GetMappedRange());
    std::memcpy(renderInfoBufferData + 0, &sampleCount, 4);
    std::memcpy(renderInfoBufferData + 4, &samplesPerDraw, 4);
    std::memcpy(renderInfoBufferData + 8, &numDraws, 4);
    renderInfoBuffer.Unmap();

    bindGroupEntries[3].binding = 3;
    bindGroupEntries[3].buffer = renderInfoBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();
    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

    std::vector<std::string> completeMessages((roughnessMipLevels - 1) * numDraws);

    std::cout << "computing pre filter..." << std::endl;

    int drawIndex = 0;
    for (int mipLevel = roughnessMipLevels - 1; mipLevel >= 1; mipLevel--) {
        float roughness = static_cast<float>(mipLevel) / (static_cast<float>(roughnessMipLevels) - 1.0f);
        device.GetQueue().WriteBuffer(roughnessBuffer, 0, &roughness, 4);

        wgpu::TextureViewDescriptor outputTextureViewDescriptor;
        outputTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        outputTextureViewDescriptor.baseArrayLayer = 0;
        outputTextureViewDescriptor.arrayLayerCount = 1;
        outputTextureViewDescriptor.baseMipLevel = mipLevel;
        outputTextureViewDescriptor.mipLevelCount = 1;

        colorAttachment.view = equirectangularTextureOutput.CreateView(&outputTextureViewDescriptor);

        for(uint32_t draw = 0; draw < numDraws; draw++) {
            auto encoder = device.CreateCommandEncoder();

            auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

            renderPassEncoder.SetPipeline(shader.renderPipeline(false));
            renderPassEncoder.SetBindGroup(0, bindGroup);

            renderPassEncoder.Draw(3, 1, 0, draw);

            renderPassEncoder.End();

            auto commandBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &commandBuffer);

            float percentageFloat = static_cast<float>(drawIndex + 1) / static_cast<float>((roughnessMipLevels - 1) * numDraws);
            percentageFloat = percentageFloat * percentageFloat;
            int percentageInt = static_cast<int>(percentageFloat * 100.0f);
            if(percentageInt == 0) {
                percentageInt = 1;
            }
            std::string message = std::to_string(percentageInt) + "% pre filter. pass (" + std::to_string(drawIndex + 1) + "/" + std::to_string((roughnessMipLevels - 1) * numDraws) + ")";
            completeMessages[drawIndex] = message;
            device.GetQueue().OnSubmittedWorkDone([](WGPUQueueWorkDoneStatus status, void * userdata) {
                std::cout << *reinterpret_cast<std::string*>(userdata) << std::endl;
            }, &completeMessages[drawIndex]);

            device.Tick();
            drawIndex++;
        }
    }

    wgpu::BufferDescriptor descriptor;
    descriptor.size = equirectangularTexture.size().width * equirectangularTexture.size().width * 16;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    std::ostringstream outputStream;

    outputStream << GameEngine::Random::uuid();

    std::string imageType = "hdr";
    outputStream.write(imageType.c_str(), imageType.size() + 1);

    uint32_t mipLevelsInFile = roughnessMipLevels;
    outputStream.write(reinterpret_cast<char *>(&mipLevelsInFile), sizeof(uint32_t));

    {
        int x, y;
        float *imageFloats = stbi_loadf(inputFilePath.string().c_str(), &x, &y, nullptr, 3);

        GameEngine::clearImageDataBuffer();

        stbi_write_hdr_to_func(GameEngine::writeImageDataToBuffer, nullptr, static_cast<int>(x), static_cast<int>(y), 3, imageFloats);

        uint32_t imageNumBytes = GameEngine::imageDataBuffer().size();
        outputStream.write(reinterpret_cast<const char *>(&imageNumBytes), sizeof(uint32_t));

        outputStream.write(reinterpret_cast<const char *>(GameEngine::imageDataBuffer().data()), GameEngine::imageDataBuffer().size());
    }

    for (uint32_t level = 1; level < roughnessMipLevels; level++) {
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
            exit(1);
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

        GameEngine::clearImageDataBuffer();

        stbi_write_hdr_to_func(GameEngine::writeImageDataToBuffer, nullptr, static_cast<int>(mipWidth), static_cast<int>(mipHeight), 3, imageFloats.data());

        uint32_t imageNumBytes = GameEngine::imageDataBuffer().size();
        outputStream.write(reinterpret_cast<const char *>(&imageNumBytes), sizeof(uint32_t));

        outputStream.write(reinterpret_cast<const char *>(GameEngine::imageDataBuffer().data()), GameEngine::imageDataBuffer().size());

        readBackBuffer.Unmap();
    }

    return outputStream;
}
