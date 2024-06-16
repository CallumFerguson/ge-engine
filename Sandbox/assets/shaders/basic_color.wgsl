//3284227e-817a-4bf6-b184-8cbb3b15d503

const PI = 3.14159265359;

struct CameraData {
    view: mat4x4f,
    projection: mat4x4f,
    position: vec3f,
    viewDirectionProjectionInverse: mat4x4f,
}

struct ObjectData {
    model: mat4x4f,
    color: vec4f,
//    normalMatrix: mat4x4f,
//    metallic: f32,
//    roughness: f32,
}

@group(0) @binding(0) var<uniform> cameraData: CameraData;

@group(1) @binding(0) var textureSampler: sampler;
@group(1) @binding(1) var albedoTexture: texture_2d<f32>;
@group(1) @binding(2) var normalTexture: texture_2d<f32>;
@group(1) @binding(3) var occlusionRoughnessMetalicTexture: texture_2d<f32>;
//@group(1) @binding(4) var emissionTexture: texture_2d<f32>;
//@group(1) @binding(5) var brdfLUTTexture: texture_2d<f32>;
//@group(1) @binding(5) var environmentIrradianceCubeMapTexture: texture_cube<f32>;
//@group(1) @binding(6) var environmentPrefilterCubeMapTexture: texture_cube<f32>;

//@group(2) @binding(0) var<storage, read> objectData: array<ObjectData>;
@group(2) @binding(0) var<uniform> objectData: ObjectData;

struct VertexInput {
    @builtin(instance_index) instanceIndex: u32,
    @location(0) position: vec4f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) tangent: vec4f,
}

struct VertexOutput {
    @builtin(position) fragPosition: vec4f,
    @location(0) worldPosition: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
    @location(5) fp: vec2f,
//    @location(0) metallic: f32,
//    @location(0) roughness: f32,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    var o: VertexOutput;

    o.fragPosition = cameraData.projection * cameraData.view * objectData.model * i.position;
    o.worldPosition = (objectData.model * i.position).xyz;
    o.normal = normalize((objectData.model * vec4(i.normal, 0)).xyz);
    o.uv = i.uv;
    o.tangent = normalize((objectData.model * vec4(i.tangent.xyz, 0)).xyz);
    o.bitangent = normalize((objectData.model * vec4(i.tangent.w * cross(o.normal, o.tangent), 0)).xyz);

    o.fp = o.fragPosition.xy;

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
//    let emission = textureSample(emissionTexture, textureSampler, i.uv);
    let normal = textureSample(normalTexture, textureSampler, i.uv);
    let occlusionRoughnessMetalic = textureSample(occlusionRoughnessMetalicTexture, textureSampler, i.uv);
//    let brdfLUT = textureSample(brdfLUTTexture, textureSampler, i.uv);

    let TBN = mat3x3(i.tangent, i.bitangent, i.normal);
//    let tangentSpaceNormal = textureSample(normalTexture, textureSampler, i.uv).rgb * 2 - 1;
    let tangentSpaceNormal: vec3f = vec3(0, 0, 1);
    let worldNormal = normalize(TBN * tangentSpaceNormal);
    var light = dot(worldNormal, normalize(-vec3(0.25, -1, -0.25)));
    light = max(light, 0);
    light += 0.1;
    light = min(light, 1);
    let albedo = textureSample(albedoTexture, textureSampler, i.uv);

    if (albedo.a < 0.2) {
        discard;
    }

    if (i.fp.x < 0) {
        return vec4(albedo.rgb * light, 1.0f);
    } else {
        return vec4(normal.rgb, 1.0f);
    }
}
