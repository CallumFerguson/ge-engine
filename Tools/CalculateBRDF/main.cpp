#include <iostream>
#include <fstream>
#include <string>
#include <stb_image_write.h>
#include "GameEngine.hpp"

const char* calculateBRDFShaderString = R"(
//6c1862c6-26f3-40b1-bb1f-1602fce01f05

const PI = 3.14159265359;

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    let positions = array<vec2f, 3>(
      vec2f(-1, -1),
      vec2f(3, -1),
      vec2f(-1, 3),
    );
    let uvs = array<vec2f, 3>(
      vec2f(0, 1),
      vec2f(2, 1),
      vec2f(0, -1),
    );

    var o: VertexOutput;

    o.position = vec4(positions[i.vertexIndex], 0, 1);
    o.uv = uvs[i.vertexIndex];

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return vec4(integrateBRDF(i.uv.x, i.uv.y), 0, 1);
}

fn integrateBRDF(NdotV: f32, roughness: f32) -> vec2f {
    var V: vec3f;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;

    var A = 0.0;
    var B = 0.0;

    var N = vec3(0.0, 0.0, 1.0);

    const SAMPLE_COUNT: u32 = 1024;
    for(var i: u32 = 0; i < SAMPLE_COUNT; i++)
    {
        let Xi = hammersley(i, SAMPLE_COUNT);
        let H = importanceSampleGGX(Xi, N, roughness);
        let L = normalize(2.0 * dot(V, H) * H - V);

        let NdotL = max(L.z, 0.0);
        let NdotH = max(H.z, 0.0);
        let VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            let G = geometrySmith(N, V, L, roughness);
            let G_Vis = (G * VdotH) / (NdotH * NdotV);
            let Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= f32(SAMPLE_COUNT);
    B /= f32(SAMPLE_COUNT);
    return vec2(A, B);
}

fn radicalInverse_VdC(bits: u32) -> f32 {
    var bitsOut = bits;
    bitsOut = (bitsOut << 16u) | (bitsOut >> 16u);
    bitsOut = ((bitsOut & 0x55555555u) << 1u) | ((bitsOut & 0xAAAAAAAAu) >> 1u);
    bitsOut = ((bitsOut & 0x33333333u) << 2u) | ((bitsOut & 0xCCCCCCCCu) >> 2u);
    bitsOut = ((bitsOut & 0x0F0F0F0Fu) << 4u) | ((bitsOut & 0xF0F0F0F0u) >> 4u);
    bitsOut = ((bitsOut & 0x00FF00FFu) << 8u) | ((bitsOut & 0xFF00FF00u) >> 8u);
    return f32(bitsOut) * 2.3283064365386963e-10; // / 0x100000000
}

fn hammersley(i: u32, N: u32) -> vec2f {
    return vec2(f32(i) / f32(N), radicalInverse_VdC(i));
}

fn importanceSampleGGX(Xi: vec2f, N: vec3f, roughness: f32) -> vec3f{
    let a = roughness * roughness;

    let phi = 2.0 * PI * Xi.x;
    let cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    let sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    var H: vec3f;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    let up = select(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), abs(N.z) < 0.999);
    let tangent = normalize(cross(up, N));
    let bitangent = cross(N, tangent);

    let sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

fn geometrySmith(N: vec3f, V: vec3f, L: vec3f, roughness: f32) -> f32 {
    let NdotV = max(dot(N, V), 0.0);
    let NdotL = max(dot(N, L), 0.0);
    let ggx2 = geometrySchlickGGXForBRDF(NdotV, roughness);
    let ggx1 = geometrySchlickGGXForBRDF(NdotL, roughness);

    return ggx1 * ggx2;
}

fn geometrySchlickGGXForBRDF(NdotV: f32, roughness: f32) -> f32 {
  let a = roughness;
  let k = (a * a) / 2.0;

  let num = NdotV;
  let denom = NdotV * (1.0 - k) + k;

  return num / denom;
}
)";

static std::vector<uint8_t> s_stbImageWriteBuffer;

void writeImageDataToFile(void *context, void *data, int size) {
    auto byteData = reinterpret_cast<const char *>(data);
    s_stbImageWriteBuffer.insert(s_stbImageWriteBuffer.end(), byteData, byteData + size);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <resolution> <output_file_path>" << std::endl;
        return 1;
    }

    std::string resolutionString = argv[1];
    int resolution = std::stoi(resolutionString);

    if(resolution < 256) {
        std::cout << "resolution must be at least 256" << std::endl;
        return 1;
    }

    std::filesystem::path outputFilePath(argv[2]);
    outputFilePath /= "BRDF.getexture";

    GameEngine::WebGPURenderer::init(nullptr);

    auto &device = GameEngine::WebGPURenderer::device();

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = nullptr; // auto layout

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = calculateBRDFShaderString;

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.nextInChain = &wgslDescriptor;

    auto shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

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

    auto pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(resolution), static_cast<uint32_t>(resolution), 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    auto texture = device.CreateTexture(&textureDescriptor);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = resolution * resolution * 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};
    colorAttachment.view = texture.CreateView();

    wgpu::RenderPassDescriptor renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    auto commandEncoder = device.CreateCommandEncoder();
    auto renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);

    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.Draw(3);

    renderPassEncoder.End();

    wgpu::ImageCopyTexture src = {};
    src.texture = texture;

    wgpu::ImageCopyBuffer dst = {};
    dst.buffer = readBackBuffer;
    dst.layout.offset = 0;
    dst.layout.bytesPerRow = resolution * 4;
    dst.layout.rowsPerImage = resolution;

    wgpu::Extent3D extent = {static_cast<uint32_t>(resolution), static_cast<uint32_t>(resolution), 1};
    commandEncoder.CopyTextureToBuffer(&src, &dst, &extent);

    auto commandBuffer = commandEncoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);

    readBackBuffer.MapAsync(wgpu::MapMode::Read, 0, descriptor.size, [](WGPUBufferMapAsyncStatus status, void *userData) {
        if (status != WGPUBufferMapAsyncStatus_Success) {
            std::cout << "Failed to map buffer for reading." << std::endl;
        }
    }, nullptr);

    while (readBackBuffer.GetMapState() != wgpu::BufferMapState::Mapped) {
        device.Tick();
    }

    auto *data = reinterpret_cast<const uint8_t *>(readBackBuffer.GetConstMappedRange());
    if (!data) {
        std::cout << "no mapped range data!" << std::endl;
        return 1;
    }

//    std::string outputFilePathString = outputFilePath.string();
//    stbi_write_png(outputFilePathString.c_str(), resolution, resolution, 4, data, resolution * 4);

    std::ofstream outputFile(outputFilePath, std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return 1;
    }

    std::string uuid = BRDF_UUID;
    outputFile << uuid;

    std::string imageType = "png";
    outputFile.write(imageType.c_str(), imageType.size() + 1);

    bool hasMipLevels = false;
    outputFile.write(reinterpret_cast<char *>(&hasMipLevels), 1);

    stbi_write_png_to_func(writeImageDataToFile, &outputFile, resolution, resolution, 4, data, resolution * 4);

    uint32_t imageNumBytes = s_stbImageWriteBuffer.size();
    outputFile.write(reinterpret_cast<const char *>(&imageNumBytes), sizeof(uint32_t));

    outputFile.write(reinterpret_cast<const char *>(s_stbImageWriteBuffer.data()), s_stbImageWriteBuffer.size());

    readBackBuffer.Unmap();
}
