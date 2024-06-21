#include "generateMipmapWebGPU.hpp"

// adapted from: https://github.com/greggman/webgpu-utils/blob/3420af728cd7cbf42773dc5d2db428185852c53d/src/generate-mipmap.ts

#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

wgpu::TextureViewDimension guessTextureBindingViewDimensionForTexture(const wgpu::Texture &texture) {
    switch (texture.GetDimension()) {
        case wgpu::TextureDimension::e1D:
            return wgpu::TextureViewDimension::e1D;
        case wgpu::TextureDimension::e3D:
            return wgpu::TextureViewDimension::e3D;
        default:
        case wgpu::TextureDimension::e2D:
            return texture.GetDepthOrArrayLayers() > 1 ? wgpu::TextureViewDimension::e2DArray : wgpu::TextureViewDimension::e2D;
    }
}

uint32_t numMipLevels(const wgpu::Extent3D &size) {
    uint32_t maxSize = std::max({size.width, size.height, size.depthOrArrayLayers});
    return 1 + static_cast<uint32_t>(std::floor(std::log2(maxSize)));
}

std::string getMipmapGenerationWGSL(wgpu::TextureViewDimension textureBindingViewDimension) {
    std::string textureSnippet;
    std::string sampleSnippet;
    switch (textureBindingViewDimension) {
        case wgpu::TextureViewDimension::e2D:
            textureSnippet = "texture_2d<f32>";
            sampleSnippet = "textureSample(ourTexture, ourSampler, fsInput.texcoord)";
            break;
        case wgpu::TextureViewDimension::e2DArray:
            textureSnippet = "texture_2d_array<f32>";
            sampleSnippet = R"(
                textureSample(
                    ourTexture,
                    ourSampler,
                    fsInput.texcoord,
                    uni.layer
                )
            )";
            break;
        case wgpu::TextureViewDimension::Cube:
            textureSnippet = "texture_cube<f32>";
            sampleSnippet = R"(
                textureSample(
                    ourTexture,
                    ourSampler,
                    faceMat[uni.layer] * vec3f(fract(fsInput.texcoord), 1)
                )
            )";
            break;
        case wgpu::TextureViewDimension::CubeArray:
            textureSnippet = "texture_cube_array<f32>";
            sampleSnippet = R"(
                textureSample(
                    ourTexture,
                    ourSampler,
                    faceMat[uni.layer] * vec3f(fract(fsInput.texcoord), 1),
                    uni.layer
                )
            )";
            break;
        default:
            throw std::runtime_error("unsupported view: " + std::to_string(static_cast<int>(textureBindingViewDimension)));
    }
    return R"(
        const faceMat = array(
            mat3x3f(0, 0, -2, 0, -2, 0, 1, 1, 1),   // pos-x
            mat3x3f(0, 0, 2, 0, -2, 0, -1, 1, -1),  // neg-x
            mat3x3f(2, 0, 0, 0, 0, 2, -1, 1, -1),   // pos-y
            mat3x3f(2, 0, 0, 0, 0, -2, -1, -1, 1),  // neg-y
            mat3x3f(2, 0, 0, 0, -2, 0, -1, 1, 1),   // pos-z
            mat3x3f(-2, 0, 0, 0, -2, 0, 1, 1, -1)   // neg-z
        );

        struct VSOutput {
            @builtin(position) position: vec4f,
            @location(0) texcoord: vec2f,
        };

        @vertex fn vs(@builtin(vertex_index) vertexIndex : u32) -> VSOutput {
            var pos = array<vec2f, 3>(
                vec2f(-1.0, -1.0),
                vec2f(-1.0, 3.0),
                vec2f(3.0, -1.0)
            );

            var vsOutput: VSOutput;
            let xy = pos[vertexIndex];
            vsOutput.position = vec4f(xy, 0.0, 1.0);
            vsOutput.texcoord = xy * vec2f(0.5, -0.5) + vec2f(0.5);
            return vsOutput;
        }

        struct Uniforms {
            layer: u32,
        };

        @group(0) @binding(0) var ourSampler: sampler;
        @group(0) @binding(1) var ourTexture: )" + textureSnippet + R"(;
        @group(0) @binding(2) var<uniform> uni: Uniforms;

        @fragment fn fs(fsInput: VSOutput) -> @location(0) vec4f {
            _ = uni.layer; // make sure this is used so all pipelines have the same bindings
            return )" + sampleSnippet + R"(;
        }
    )";
}

struct PerDeviceInfo {
    std::map<std::string, wgpu::RenderPipeline> pipelineByFormatAndView;
    std::map<wgpu::TextureViewDimension, wgpu::ShaderModule> moduleByViewType;
    wgpu::Sampler sampler;
    wgpu::Buffer uniformBuffer;
    uint32_t uniformValues = 0;
};

static std::map<WGPUDeviceImpl *, PerDeviceInfo> s_byDevice;

void generateMipmap(const wgpu::Device &device, const wgpu::Texture &texture, wgpu::TextureViewDimension textureBindingViewDimension) {
    if (!s_byDevice.contains(device.Get())) {
        s_byDevice[device.Get()] = {};
    }

    auto &perDeviceInfo = s_byDevice[device.Get()];

    if (textureBindingViewDimension == wgpu::TextureViewDimension::Undefined) {
        textureBindingViewDimension = guessTextureBindingViewDimensionForTexture(texture);
    }

    if (!perDeviceInfo.moduleByViewType.contains(textureBindingViewDimension)) {
        std::string code = getMipmapGenerationWGSL(textureBindingViewDimension);

        wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
        wgslDescriptor.code = code.c_str();

        wgpu::ShaderModuleDescriptor shaderModuleDesc;
        shaderModuleDesc.nextInChain = &wgslDescriptor;

        perDeviceInfo.moduleByViewType[textureBindingViewDimension] = device.CreateShaderModule(&shaderModuleDesc);
    }
    auto &module = perDeviceInfo.moduleByViewType[textureBindingViewDimension];

    if (!perDeviceInfo.sampler) {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.minFilter = wgpu::FilterMode::Linear;
        samplerDesc.magFilter = wgpu::FilterMode::Linear;
        perDeviceInfo.sampler = device.CreateSampler(&samplerDesc);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = 16;
        bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        perDeviceInfo.uniformBuffer = device.CreateBuffer(&bufferDesc);
    }

    std::string id = std::to_string(static_cast<uint32_t>(texture.GetFormat())) + "." + std::to_string(static_cast<uint32_t>(textureBindingViewDimension));

    if (!perDeviceInfo.pipelineByFormatAndView.contains(id)) {
        wgpu::RenderPipelineDescriptor pipelineDesc;

        pipelineDesc.layout = nullptr; // auto layout

        pipelineDesc.vertex.module = module;
        pipelineDesc.vertex.entryPoint = "vs";

        wgpu::ColorTargetState colorTarget;
        colorTarget.format = texture.GetFormat();

        wgpu::FragmentState fragment = {};
        fragment.module = module;
        fragment.entryPoint = "fs";
        fragment.targets = &colorTarget;
        fragment.targetCount = 1;

        pipelineDesc.fragment = &fragment;

        perDeviceInfo.pipelineByFormatAndView[id] = device.CreateRenderPipeline(&pipelineDesc);
    }
    auto& pipeline = perDeviceInfo.pipelineByFormatAndView[id];

    for (uint32_t baseMipLevel = 1; baseMipLevel < texture.GetMipLevelCount(); baseMipLevel++) {
        for (uint32_t baseArrayLayer = 0; baseArrayLayer < texture.GetDepthOrArrayLayers(); baseArrayLayer++) {
            perDeviceInfo.uniformValues = baseArrayLayer;
            device.GetQueue().WriteBuffer(perDeviceInfo.uniformBuffer, 0, &perDeviceInfo.uniformValues, sizeof(uint32_t));

            wgpu::BindGroupEntry entry0;
            entry0.binding = 0;
            entry0.sampler = perDeviceInfo.sampler;

            wgpu::TextureViewDescriptor textureViewDescriptor;
            textureViewDescriptor.dimension = textureBindingViewDimension;
            textureViewDescriptor.baseMipLevel = baseMipLevel - 1;
            textureViewDescriptor.mipLevelCount = 1;
            textureViewDescriptor.baseArrayLayer = baseArrayLayer;
            textureViewDescriptor.arrayLayerCount = 1;

            wgpu::BindGroupEntry entry1;
            entry1.binding = 1;
            entry1.textureView = texture.CreateView(&textureViewDescriptor);

            wgpu::BindGroupEntry entry2;
            entry2.binding = 2;
            entry2.buffer = perDeviceInfo.uniformBuffer;

            std::vector<wgpu::BindGroupEntry> entries = {entry0, entry1, entry2};

            wgpu::BindGroupDescriptor bindGroupDesc;
            bindGroupDesc.layout = pipeline.GetBindGroupLayout(0);
            bindGroupDesc.entries = entries.data();
            bindGroupDesc.entryCount = entries.size();
            auto bindGroup = device.CreateBindGroup(&bindGroupDesc);

            wgpu::TextureViewDescriptor textureViewDescriptor2;
            textureViewDescriptor2.dimension = wgpu::TextureViewDimension::e2D;
            textureViewDescriptor2.baseMipLevel = baseMipLevel;
            textureViewDescriptor2.mipLevelCount = 1;
            textureViewDescriptor2.baseArrayLayer = baseArrayLayer;
            textureViewDescriptor2.arrayLayerCount = 1;

            wgpu::RenderPassColorAttachment colorAttachment;
            colorAttachment.view = texture.CreateView(&textureViewDescriptor2);
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

            wgpu::RenderPassDescriptor renderPassDesc;
            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &colorAttachment;

            auto encoder = device.CreateCommandEncoder();

            auto pass = encoder.BeginRenderPass(&renderPassDesc);

            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();

            auto commandBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &commandBuffer);
        }
    }
}

}
