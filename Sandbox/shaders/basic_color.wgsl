struct CameraData {
    view: mat4x4f,
    projection: mat4x4f,
}

struct ObjectData {
    model: mat4x4f,
    color: vec4f,
}

@group(0) @binding(0) var<uniform> cameraData: CameraData;

@group(1) @binding(0) var<uniform> objectData: ObjectData;

struct VertexInput {
    @location(0) position: vec4f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) tangent: vec4f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) tangent: vec3f,
    @location(1) bitangent: vec3f,
    @location(2) normal: vec3f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    var o: VertexOutput;

    o.position = cameraData.projection * cameraData.view * objectData.model * i.position;
    o.normal = normalize((objectData.model * vec4(i.normal, 0)).xyz);
    o.tangent = normalize((objectData.model * vec4(i.tangent.xyz, 0)).xyz);
    o.bitangent = normalize((objectData.model * vec4(i.tangent.w * cross(o.normal, o.tangent), 0)).xyz);

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    let TBN = mat3x3(i.tangent, i.bitangent, i.normal);
//    let tangentSpaceNormal = textureSample(normalTexture, textureSampler, i.uv).rgb * 2 - 1;
    let tangentSpaceNormal: vec3f = vec3(0, 0, 1);
    let worldNormal = normalize(TBN * tangentSpaceNormal);
    var light = dot(worldNormal, normalize(-vec3(0.25, -1, -0.25)));
    light = max(light, 0);
    light += 0.1;
    light = min(light, 1);
    return vec4(light, light, light, 1.0f);
}
