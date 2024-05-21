@group(0) @binding(0) var<uniform> color: vec4f;

struct VertexInput {
    @location(0) position: vec4f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    var o: VertexOutput;

    o.position = i.position;

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return color;
}
