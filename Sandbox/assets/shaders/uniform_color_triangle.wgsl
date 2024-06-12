//a5694f7d-2c6e-441c-be46-2aac844e7bd4

@group(0) @binding(0) var<uniform> color: vec4f;

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    let positions = array<vec2f, 3>(
      vec2f(0, 1),
      vec2f(-1, -1),
      vec2f(1, -1),
    );

    var o: VertexOutput;

    o.position = vec4(positions[i.vertexIndex], 0, 1);

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return color;
}
