//c14e9db1-a5e9-46d9-99d2-498a8869dd38

const PI = 3.14159265359;

struct RenderInfo {
    sampleCount: u32,
    samplesPerDraw: u32,
    numDraws: u32,
}

@group(0) @binding(0) var textureSampler: sampler;
@group(0) @binding(1) var texture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> roughness: f32;
@group(0) @binding(3) var<uniform> renderInfo: RenderInfo;

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
    @builtin(instance_index) instanceIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
    @location(1) @interpolate(flat) instanceIndex: u32,
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

    o.position = vec4(positions[i.vertexIndex], 1, 1);
    o.uv = uvs[i.vertexIndex];
    o.instanceIndex = i.instanceIndex;

    return o;
}

const invAtan = vec2(0.1591, 0.3183);
fn directionToEquirectangularCoordinates(v: vec3f) -> vec2f {
    var uv = vec2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

const invAtanInv = vec2(1.0 / 0.1591, 1.0 / 0.3183);
fn equirectangularCoordinatesToDirection(uv: vec2f) -> vec3f {
    var uv_transformed = (uv - 0.5) * invAtanInv;
    var theta = uv_transformed.x;
    var phi = uv_transformed.y;

    var x = cos(phi) * cos(theta);
    var y = sin(phi);
    var z = cos(phi) * sin(theta);

    return normalize(vec3(x, y, z));
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    let direction = equirectangularCoordinatesToDirection(i.uv);

    let N = direction;
    let R = N;
    let V = R;

    var totalWeight = 0.0;
    var prefilteredColor = vec3(0.0);
    for(var n = 0u; n < renderInfo.samplesPerDraw; n++)
    {
        let Xi = hammersley(i.instanceIndex * renderInfo.samplesPerDraw + n, renderInfo.sampleCount);
        let H = importanceSampleGGX(Xi, N, roughness);
        let L = normalize(2.0 * dot(V, H) * H - V);

        let NdotL = max(dot(N, L), 0.0);

        // sample from the environment's mip level based on roughness/pdf
        let D = distributionGGX(N, H, roughness);
        let NdotH = max(dot(N, H), 0.0);
        let HdotV = max(dot(H, V), 0.0);
        let pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

        let resolution: f32 = 2048; // resolution of source cubemap (per face)
        let saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
        let saSample = 1.0 / (f32(renderInfo.sampleCount) * pdf + 0.0001);

        let uv = directionToEquirectangularCoordinates(L);
        let mipLevel = select(0.5 * log2(saSample / saTexel), 0.0, roughness == 0.0);
        let sampleColor = textureSampleLevel(texture, textureSampler, uv, mipLevel).rgb;

        if(NdotL > 0.0)
        {
            prefilteredColor += sampleColor * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight / f32(renderInfo.numDraws);

    return vec4(prefilteredColor, 1);
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
