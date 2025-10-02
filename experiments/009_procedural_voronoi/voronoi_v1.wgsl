struct ComputeParams {
    // Size of the texture to generate
    textureSize: vec3u,
    padding0: u32,
    // Origin point of the texture data in the output texture storage
    origin: vec3u,
    padding1: u32,
};

@group(0) @binding(0) var<uniform> params : ComputeParams;
@group(0) @binding(1) var outputTex: texture_storage_2d_array<rgba8unorm,write>;

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) id: vec3u) {

    if id.x > params.textureSize.x || id.y > params.textureSize.y {
        // Nothing to write in this case.
        return;
    }

    // Number of pixels on each dimension to form a grid cell:
    let gridSize: f32 = 64.0;

    // Get our coordinates on the grid:
    let gcoords = vec2f(id.xy) / gridSize;

    // From those coords we can extract the integer part which
    // are the cell coords, and the fractional part which are our pixel
    // local (normalized) coords on the cell:
    let cellCoords = vec2i(floor(gcoords) + 0.5);
    let localCoords = gcoords - floor(gcoords);

    // Write the localCoords as color:
    let color = vec4f(localCoords, 0.0, 1.0);

    // Write yellow color:
    // let color = vec4f(1.0, 1.0, 0.0, 1.0);

    let layer: u32 = params.origin.z;
    textureStore(outputTex, vec2i(params.origin.xy + id.xy), layer, color);
}
