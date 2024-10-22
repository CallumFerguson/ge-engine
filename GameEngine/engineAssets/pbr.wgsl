//7aa2b713-86dd-4c48-a9ea-9af110d116ee

const PI = 3.14159265359;

struct CameraData {
    view: mat4x4f,
    projection: mat4x4f,
    position: vec3f,
    exposure: f32,
    viewDirectionProjectionInverse: mat4x4f,
}

struct ObjectData {
    model: mat4x4f,
    normalMatrix: mat4x4f,
    color: vec4f,
//    metallic: f32,
//    roughness: f32,
}

// pbr environment map
@group(0) @binding(0) var textureSampler: sampler;
@group(0) @binding(1) var brdfLUTTexture: texture_2d<f32>;
@group(0) @binding(2) var environmentPrefilterCubeMapTexture: texture_cube<f32>;
@group(0) @binding(3) var environmentIrradianceCubeMapTexture: texture_cube<f32>;

// camera data
@group(1) @binding(0) var<uniform> cameraData: CameraData;

// pbr material
@group(2) @binding(0) var albedoTexture: texture_2d<f32>;
@group(2) @binding(1) var normalTexture: texture_2d<f32>;
@group(2) @binding(2) var occlusionRoughnessMetalicTexture: texture_2d<f32>;
@group(2) @binding(3) var emissiveTexture: texture_2d<f32>;

// pbr object data
@group(3) @binding(0) var<uniform> objectData: ObjectData;

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
//    @location(0) metallic: f32,
//    @location(0) roughness: f32,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
//    let objectData = objectData[i.instanceIndex];

    var o: VertexOutput;

    o.fragPosition = cameraData.projection * cameraData.view * objectData.model * i.position;
    o.worldPosition = (objectData.model * i.position).xyz;
    o.normal = normalize((objectData.normalMatrix * vec4(i.normal, 0)).xyz);
    o.uv = i.uv;
    o.tangent = normalize((objectData.normalMatrix * vec4(i.tangent.xyz, 0)).xyz);
    o.bitangent = normalize((objectData.normalMatrix * vec4(i.tangent.w * cross(o.normal, o.tangent), 0)).xyz);
//    o.metallic = objectData.metallic;
//    o.roughness = objectData.roughness;

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    const gamma: f32 = 2.2;

    let albedoTexel = textureSample(albedoTexture, textureSampler, i.uv);
    let albedo = pow(albedoTexel.rgb * objectData.color.rgb, vec3(gamma));
//    let albedo: vec3f = vec3(0.5, 0, 0);
//    let albedo: vec3f = vec3(1, 1, 1);

    let emission = pow(textureSample(emissiveTexture, textureSampler, i.uv).rgb, vec3(gamma));
//    let emission = vec3f(0, 0, 0);

    let occlusionRoughnessMetalic = textureSample(occlusionRoughnessMetalicTexture, textureSampler, i.uv).rgb;
//    let occlusionRoughnessMetalic: vec3f = vec3(1, i.roughness, i.metallic);
//    let occlusionRoughnessMetalic: vec3f = vec3(1, 1, 1);

    let TBN = mat3x3(i.tangent, i.bitangent, i.normal);
    let tangentSpaceNormal = textureSample(normalTexture, textureSampler, i.uv).rgb * 2 - 1;
//    let tangentSpaceNormal: vec3f = vec3(0, 0, 1);
    let worldNormal = normalize(TBN * tangentSpaceNormal);

    const lightPositions = array<vec3f, 4>(
        vec3f(-10, 10, 10),
        vec3f(10, 10, 10),
        vec3f(-10, -10, 10),
        vec3f(10, -10, 10)
    );

    const lightColors = array<vec3f, 4>(
        vec3f(300, 300, 300),
        vec3f(300, 300, 300),
        vec3f(300, 300, 300),
        vec3f(300, 300, 300)
    );

    let metallic = occlusionRoughnessMetalic.b;
    let roughness = occlusionRoughnessMetalic.g;
    let ao = occlusionRoughnessMetalic.r;

    let N = worldNormal;
    let V = normalize(cameraData.position - i.worldPosition);

    var F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    var Lo = vec3(0.0);
    for (var n = 0; n < 0; n++)
    {
        // calculate per-light radiance
        let L = normalize(lightPositions[n] - i.worldPosition);
        let H = normalize(V + L);
        let distance = length(lightPositions[n] - i.worldPosition);
        let attenuation = 1.0 / (distance * distance);
        let radiance = lightColors[n] * attenuation;

        // cook-torrance brdf
        let NDF = distributionGGX(N, H, roughness);
        let G = geometrySmith(N, V, L, roughness);
        let F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        let kS = F;
        var kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        let numerator = NDF * G * F;
        let denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        let specular = numerator / denominator;

        // add to outgoing radiance Lo
        let NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    let F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    let kS = F;
    var kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    let irradiance = textureSample(environmentIrradianceCubeMapTexture, textureSampler, worldNormal * vec3f(-1, 1, 1)).rgb;
//    let irradiance = vec3f(0, 0, 0);
    let diffuse = irradiance * albedo;

    let R = reflect(-V, N);

    const MAX_REFLECTION_LOD = 4.0;
    let prefilteredColor = textureSampleLevel(environmentPrefilterCubeMapTexture, textureSampler, R * vec3f(-1, 1, 1), roughness * MAX_REFLECTION_LOD).rgb;
//    let prefilteredColor = vec3f(0, 0, 0);
    let envBRDF = textureSample(brdfLUTTexture, textureSampler, vec2(max(dot(N, V), 0.0), roughness)).rg;
    let specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    let ambient = (kD * diffuse + specular) * ao;

    var colorLinear = ambient + Lo + emission;
    colorLinear = vec3(1.0) - exp(-colorLinear * cameraData.exposure);

    let color = pow(colorLinear, vec3(1.0 / gamma));
    return vec4(color, albedoTexel.a);
}

fn pow5(value: f32) -> f32 {
    let value2 = value * value;
    let value4 = value2 * value2;
    return value4 * value;
}

fn fresnelSchlick(cosTheta: f32, F0: vec3f) -> vec3f {
    return F0 + (1.0 - F0) * pow5(clamp(1.0 - cosTheta, 0.0, 1.0));
}

fn fresnelSchlickRoughness(cosTheta: f32, F0: vec3f, roughness: f32) -> vec3f {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

fn distributionGGX(N: vec3f, H: vec3f, roughness: f32) -> f32 {
    let a = roughness * roughness;
    let a2 = a * a;
    let NdotH = max(dot(N, H), 0.0);
    let NdotH2 = NdotH * NdotH;

    let num = a2;
    var denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

fn geometrySchlickGGX(NdotV: f32, roughness: f32) -> f32 {
    let r = roughness + 1.0;
    let k = (r * r) / 8.0;

    let num = NdotV;
    let denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

fn geometrySmith(N: vec3f, V: vec3f, L: vec3f, roughness: f32) -> f32 {
    let NdotV = max(dot(N, V), 0.0);
    let NdotL = max(dot(N, L), 0.0);
    let ggx2 = geometrySchlickGGX(NdotV, roughness);
    let ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
