//f72d3012-f4ad-4f9d-b953-00866cd6f853

struct VertexInput {
    @builtin(vertex_index) vertexIndex: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
}

@vertex
fn vert(i: VertexInput) -> VertexOutput {
    let positions = array<vec2f, 3>(
      vec2f(-1, -1),
      vec2f(3, -1),
      vec2f(-1, 3),
    );

    var o: VertexOutput;

    o.position = vec4(positions[i.vertexIndex], 0, 1);

    return o;
}

@fragment
fn frag(i: VertexOutput) -> @location(0) vec4f {
    return vec4(0.25, 0, 0, 1);
}
