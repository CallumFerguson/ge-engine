//20b9adeb-c3d4-4bd4-8e0c-18e0b3238af9

@group(0) @binding(0) var<uniform> viewDirectionProjectionInverse: mat4x4f;
@group(0) @binding(1) var textureSampler: sampler;
@group(0) @binding(2) var texture: texture_2d<f32>;

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) pos: vec4f,
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

    return o;
}

fn sampleSphericalMap(v: vec3f) -> vec2f {
    const invAtan = vec2(0.1591, 0.3183);
    var uv = vec2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
//    const gamma: f32 = 2.2;
//    const exposure: f32 = 1;

    let t = viewDirectionProjectionInverse * i.pos;
    var direction = normalize(t.xyz / t.w) * vec3f(1, -1, 1);
    direction = vec3(-direction.z, direction.y, direction.x);
    let uv = sampleSphericalMap(direction);
    var colorLinear = textureSample(texture, textureSampler, uv).rgb;

//    colorLinear = vec3(1.0) - exp(-colorLinear * exposure);
//    let color = pow(colorLinear, vec3(1.0 / gamma));

    return vec4(colorLinear, 1);
}
