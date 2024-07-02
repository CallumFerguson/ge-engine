//cb03c587-a565-4da6-8325-227596aa4dcd

const PI = 3.14159265359;

struct RenderInfo {
    sampleDelta: f32,
    phiRange: f32,
    thetaRange: f32,
    numPhiSamples: i32,
    numThetaSamples: i32,
}

@group(0) @binding(0) var textureSampler: sampler;
@group(0) @binding(1) var texture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> renderInfo: RenderInfo;

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

    o.position = vec4(positions[i.vertexIndex], 0, 1);
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

    var irradiance = vec3f(0, 0, 0);

    var up = vec3(0.0, 1.0, 0.0);
    let right = normalize(cross(up, direction));
    up = normalize(cross(direction, right));

    let phiSample = i.instanceIndex;
    let phi = f32(phiSample) / f32(renderInfo.numPhiSamples) * renderInfo.phiRange;

    for(var thetaSample: i32 = 0; thetaSample < renderInfo.numThetaSamples; thetaSample++)
    {
        let theta = f32(thetaSample) / f32(renderInfo.numThetaSamples) * renderInfo.thetaRange;

        // spherical to cartesian (in tangent space)
        let tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        // tangent space to world
        let sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * direction;
        let uv = directionToEquirectangularCoordinates(sampleVec);

        let linearColorSample = textureSampleLevel(texture, textureSampler, uv, 0).rgb * cos(theta) * sin(theta);
//        irradiance += min(linearColorSample, vec3(25, 25, 25));
        irradiance += linearColorSample;
    }
    irradiance = PI * irradiance * (1.0 / f32(renderInfo.numThetaSamples)) / f32(renderInfo.numPhiSamples);

    return vec4(irradiance, 1);
}
