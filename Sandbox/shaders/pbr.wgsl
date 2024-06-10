const PI = 3.14159265359;

struct ObjectData {
    model: mat4x4f,
    normalMatrix: mat4x4f,
    metallic: f32,
    roughness: f32,
}

@group(0) @binding(0) var<uniform> cameraData: CameraData;

@group(1) @binding(0) var textureSampler: sampler;
@group(1) @binding(1) var albedoTexture: texture_2d<f32>;
@group(1) @binding(2) var emissionTexture: texture_2d<f32>;
@group(1) @binding(3) var normalTexture: texture_2d<f32>;
@group(1) @binding(4) var occlusionRoughnessMetalicTexture: texture_2d<f32>;
@group(1) @binding(5) var environmentIrradianceCubeMapTexture: texture_cube<f32>;
@group(1) @binding(6) var environmentPrefilterCubeMapTexture: texture_cube<f32>;
@group(1) @binding(7) var brdfLUT: texture_2d<f32>;

@group(2) @binding(0) var<storage, read> objectData: array<ObjectData>;

struct VertexInput {
    @builtin(instance_index) instanceIndex: u32,
    @location(0) position: vec4f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
}

struct VertexOutput {
    @builtin(position) fragPosition: vec4f,
    @location(0) worldPosition: vec3f,
    @location(1) uv: vec2f,
    @location(2) tangnet: vec3f,
    @location(3) bitangent: vec3f,
    @location(4) normal: vec3f,
    @location(5) metallic: f32,
    @location(6) roughness: f32,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    let objectData = objectData[i.instanceIndex];

    var o: VertexOutput;

    o.fragPosition = cameraData.projection * cameraData.view * objectData.model * i.position;
    o.worldPosition = (objectData.model * i.position).xyz;
    o.uv = i.uv;
    o.tangnet = normalize((objectData.normalMatrix * vec4(i.tangent, 0)).xyz);
    o.bitangent = normalize((objectData.normalMatrix * vec4(i.bitangent, 0)).xyz);
    o.normal = normalize((objectData.normalMatrix * vec4(i.normal, 0)).xyz);
    o.metallic = objectData.metallic;
    o.roughness = objectData.roughness;

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    const gamma: f32 = 2.2;
    const exposure: f32 = 1;

    let albedo = pow(textureSample(albedoTexture, textureSampler, i.uv).rgb, vec3(gamma));
//    let albedo: vec3f = vec3(0.5, 0, 0);
//    let albedo: vec3f = vec3(1, 1, 1);

    let emission = pow(textureSample(emissionTexture, textureSampler, i.uv).rgb, vec3(gamma));
//    let emission: vec3f = vec3(0, 0, 0);

    let occlusionRoughnessMetalic = textureSample(occlusionRoughnessMetalicTexture, textureSampler, i.uv).rgb;
//    let occlusionRoughnessMetalic: vec3f = vec3(1, i.roughness, i.metallic);

    let TBN = mat3x3(i.tangnet, i.bitangent, i.normal);
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
    let diffuse = irradiance * albedo;

    let R = reflect(-V, N);

    const MAX_REFLECTION_LOD = 4.0;
    var prefilteredColor = textureSampleLevel(environmentPrefilterCubeMapTexture, textureSampler, R * vec3f(-1, 1, 1), roughness * MAX_REFLECTION_LOD).rgb;
    let envBRDF = textureSample(brdfLUT, textureSampler, vec2(max(dot(N, V), 0.0), roughness)).rg;
    let specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    let ambient = (kD * diffuse + specular) * ao;

    var colorLinear = ambient + Lo + emission;
    colorLinear = vec3(1.0) - exp(-colorLinear * exposure);

    let color = pow(colorLinear, vec3(1.0 / gamma));
    return vec4(color, 1);
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
