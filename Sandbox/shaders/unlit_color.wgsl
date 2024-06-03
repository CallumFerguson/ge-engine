struct CameraData {
    view: mat4x4f,
    projection: mat4x4f,
}

@group(0) @binding(0) var<uniform> cameraData: CameraData;
@group(1) @binding(0) var<uniform> color: vec4f;

struct VertexInput {
    @location(0) position: vec4f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    var o: VertexOutput;

    o.position = cameraData.projection * cameraData.view * (i.position + vec4(0, 0, -2.5, 0));

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return color;
}
