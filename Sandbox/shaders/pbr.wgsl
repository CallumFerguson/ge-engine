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
}

struct VertexOutput {
    @builtin(position) position: vec4f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    var o: VertexOutput;

    o.position = cameraData.projection * cameraData.view * objectData.model * i.position;

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return objectData.color;
}
