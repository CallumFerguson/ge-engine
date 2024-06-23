//cb03c587-a565-4da6-8325-227596aa4dcd

@group(0) @binding(0) var textureSampler: sampler;
@group(0) @binding(1) var texture: texture_2d<f32>;

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
    return textureSample(texture, textureSampler, i.uv);
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

//const PI = 3.14159265359;
//
//@group(0) @binding(0) var texture: texture_cube<f32>;
//@group(0) @binding(1) var textureSampler: sampler;
//
//@group(1) @binding(0) var<uniform> cameraData: CameraData;
//
//struct VertexInput {
//    @builtin(vertex_index) vertexIndex: u32,
//}
//
//struct VertexOutput {
//    @builtin(position) position: vec4f,
//    @location(0) pos: vec4f,
//}
//
//@vertex
//fn vert(i: VertexInput) -> VertexOutput {
//    let positions = array<vec2f, 3>(
//      vec2f(-1, 3),
//      vec2f(-1,-1),
//      vec2f( 3,-1),
//    );
//
//    var o: VertexOutput;
//
//    o.position = vec4(positions[i.vertexIndex], 1, 1);
//    o.pos = o.position;
//
//    return o;
//}
//
//@fragment
//fn frag(i: VertexOutput) -> @location(0) vec4f {
//    const gamma: f32 = 2.2;
//    const exposure: f32 = 1;
//
//    let t = cameraData.viewDirectionProjectionInverse * i.pos;
//    let direction = normalize(t.xyz / t.w) * vec3f(-1, 1, 1);
//
//    var irradiance = vec3f(0, 0, 0);
//
//    var up = vec3(0.0, 1.0, 0.0);
//    let right = normalize(cross(up, direction));
//    up = normalize(cross(direction, right));
//
//    const sampleDelta = 0.025;
//    var nrSamples = 0;
//    for(var phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
//    {
//        for(var theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
//        {
//            // spherical to cartesian (in tangent space)
//            let tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
//            // tangent space to world
//            let sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * direction;
//
//            let linearColorSample = textureSample(texture, textureSampler, sampleVec).rgb * cos(theta) * sin(theta);
//            irradiance += min(linearColorSample, vec3(25, 25, 25));
//            nrSamples++;
//        }
//    }
//    irradiance = PI * irradiance * (1.0 / f32(nrSamples));
//
//    return vec4(irradiance, 1);
//}
