//20b9adeb-c3d4-4bd4-8e0c-18e0b3238af9

@group(0) @binding(0) var<uniform> viewDirectionProjectionInverses: array<mat4x4f, 6>;
@group(0) @binding(1) var textureSampler: sampler;
@group(0) @binding(2) var texture: texture_2d<f32>;

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
    @builtin(instance_index) instanceIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) pos: vec4f,
    @location(1) @interpolate(flat) instanceIndex: u32,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    let positions = array<vec2f, 3>(
      vec2f(-1, 3),
      vec2f(-1,-1),
      vec2f( 3,-1),
    );

    var o: VertexOutput;

    o.position = vec4(positions[i.vertexIndex], 1, 1);
    o.pos = o.position;
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
//    const gamma: f32 = 2.2;
//    const exposure: f32 = 1;

    let mipLevel: u32 = i.instanceIndex / 6;
    let cubeSide: u32 = i.instanceIndex % 6;

    let t = viewDirectionProjectionInverses[cubeSide] * i.pos;
    var direction = normalize(t.xyz / t.w) * vec3f(1, -1, 1);
    direction = vec3(-direction.z, direction.y, direction.x);
    let uv = directionToEquirectangularCoordinates(direction);
    var colorLinear = textureSampleLevel(texture, textureSampler, uv, f32(mipLevel)).rgb;

//    colorLinear = vec3(1.0) - exp(-colorLinear * exposure);
//    let color = pow(colorLinear, vec3(1.0 / gamma));

    return vec4(colorLinear, 1);
}
