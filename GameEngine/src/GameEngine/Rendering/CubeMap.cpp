#include "CubeMap.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Assets/AssetManager.hpp"
#include "../Utility/TimingHelper.hpp"

namespace GameEngine {

static const int cubeMapFacePixelLength = 2048;

static wgpu::Buffer s_viewDirectionProjectionInversesBuffer;

CubeMap::CubeMap(int equirectangularTextureHandle) {
    if (!s_viewDirectionProjectionInversesBuffer) {
        s_viewDirectionProjectionInversesBuffer = createViewDirectionProjectionInversesBuffer();
    }

    auto &device = WebGPURenderer::device();

    auto &equirectangularTexture = AssetManager::getAsset<Texture>(equirectangularTextureHandle);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.mipLevelCount = equirectangularTexture.mipLevelCount();
    textureDescriptor.size = {cubeMapFacePixelLength, cubeMapFacePixelLength, 6};
    textureDescriptor.format = wgpu::TextureFormat::RGBA16Float;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    *m_cubeMapTexture = device.CreateTexture(&textureDescriptor);

    std::weak_ptr<wgpu::Texture> cubeMapTextureWeak = m_cubeMapTexture;
    AssetManager::getAsset<Texture>(equirectangularTextureHandle).setReadyCallback([
                                                                                           cubeMapTextureWeak = std::move(cubeMapTextureWeak),
                                                                                           equirectangularTextureHandle]() {
        auto cubeMapTexture = cubeMapTextureWeak.lock();
        if (!cubeMapTexture) {
            return;
        }

        writeCubeMapFromEquirectangularTexture(equirectangularTextureHandle, *cubeMapTexture);
    });
}

void CubeMap::writeCubeMapFromEquirectangularTexture(int equirectangularTextureHandle, wgpu::Texture &cubeMapTexture) {
    auto &device = WebGPURenderer::device();

    const auto shaderUUID = EQUIRECTANGULAR_SKYBOX_SHADER_UUID;
    if (!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            return WebGPURenderer::createBasicPipeline(shaderModule, false, false, wgpu::TextureFormat::RGBA16Float);
        });
    }
    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    std::array<wgpu::BindGroupEntry, 3> entries;
    entries[0].binding = 0;
    entries[0].buffer = s_viewDirectionProjectionInversesBuffer;

    entries[1].binding = 1;
    entries[1].sampler = WebGPURenderer::basicSampler();

    auto &equirectangularTexture = AssetManager::getAsset<Texture>(equirectangularTextureHandle);
    entries[2].binding = 2;
    entries[2].textureView = equirectangularTexture.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor;
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = entries.size();
    bindGroupDescriptor.entries = entries.data();
    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

    auto encoder = device.CreateCommandEncoder();

    for (size_t mipLevel = 0; mipLevel < equirectangularTexture.mipLevelCount(); mipLevel++) {
        for (size_t cubeSide = 0; cubeSide < 6; cubeSide++) {
            wgpu::TextureViewDescriptor textureViewDescriptor;
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
            textureViewDescriptor.baseArrayLayer = cubeSide;
            textureViewDescriptor.arrayLayerCount = 1;
            textureViewDescriptor.baseMipLevel = mipLevel;
            textureViewDescriptor.mipLevelCount = 1;

            wgpu::RenderPassColorAttachment colorAttachment;
            colorAttachment.view = cubeMapTexture.CreateView(&textureViewDescriptor);
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

            wgpu::RenderPassDescriptor renderPassDescriptor;
            renderPassDescriptor.colorAttachmentCount = 1;
            renderPassDescriptor.colorAttachments = &colorAttachment;

            auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

            renderPassEncoder.SetPipeline(shader.renderPipeline(false));
            renderPassEncoder.SetBindGroup(0, bindGroup);
            renderPassEncoder.Draw(3, 1, 0, mipLevel * 6 + cubeSide);
            renderPassEncoder.End();
        }
    }

    auto commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);
}

wgpu::Texture &CubeMap::texture() {
    return *m_cubeMapTexture;
}

wgpu::TextureView &CubeMap::cachedTextureView() {
    if (!m_textureView) {
        wgpu::TextureViewDescriptor textureViewDescriptor;
        textureViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
        m_textureView = m_cubeMapTexture->CreateView(&textureViewDescriptor);
    }
    return m_textureView;
}

wgpu::Buffer CubeMap::createViewDirectionProjectionInversesBuffer() {
    static const auto projection = glm::perspectiveRH_ZO(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);

    static const std::array<glm::mat4, 6> views = {
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0))
    };

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.mappedAtCreation = true;
    bufferDescriptor.size = 64 * 6;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform;
    auto buffer = WebGPURenderer::device().CreateBuffer(&bufferDescriptor);
    auto mappedData = reinterpret_cast<uint8_t *>(buffer.GetMappedRange());

    for (size_t i = 0; i < 6; i++) {
        auto viewAtOrigin = views[i];
        viewAtOrigin[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        auto viewDirectionProjectionInverse = glm::inverse(projection * viewAtOrigin);

        std::memcpy(mappedData + i * 64, glm::value_ptr(viewDirectionProjectionInverse), 64);
    }

    buffer.Unmap();

    return buffer;
}

}
