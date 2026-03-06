struct ComputeParams {
    // Size of the texture to generate
    textureSize: vec3u,
    // Number of pixels on each dimension to form a grid cell:
    gridSize: f32,
    // Origin point of the texture data in the output texture storage
    origin: vec3u,
    // Border size for the grid display in number of pixels
    borderSize: f32,
    // Reference ponit size in normalized space:
    refPointSize: f32,
};

@group(0) @binding(0) var<uniform> params : ComputeParams;
@group(0) @binding(1) var outputTex: texture_storage_2d_array<rgba8unorm,write>;

fn hash(p: vec2i) -> vec2f {
    // Generate two different hash values from the 2D input
    // Using different prime coefficients for each component
    var n1 = p.x * 1597 + p.y * 3571;
    var n2 = p.x * 2179 + p.y * 5231;

    // First hash component
    var state1: u32 = u32(n1) * 747796405u + 2891336453u;
    state1 = ((state1 >> ((state1 >> 28u) + 4u)) ^ state1) * 277803737u;
    state1 = (state1 >> 22u) ^ state1;

    // Second hash component with different constants
    var state2: u32 = u32(n2) * 1103515245u + 12345u;
    state2 = ((state2 >> ((state2 >> 28u) + 4u)) ^ state2) * 277803737u;
    state2 = (state2 >> 22u) ^ state2;

    // Convert to [-1, 1] range for both components
    var x = -1.0 + 2.0 * f32(state1) / f32(0xffffffffu);
    var y = -1.0 + 2.0 * f32(state2) / f32(0xffffffffu);

    return vec2f(x, y);
}

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) id: vec3u) {

    if id.x > params.textureSize.x || id.y > params.textureSize.y {
        // Nothing to write in this case.
        return;
    }

    // Get our coordinates on the grid:
    let gcoords = vec2f(id.xy) / params.gridSize;

    // From those coords we can extract the integer part which
    // are the cell coords, and the fractional part which are our pixel
    // local (normalized) coords on the cell:
    let cellCoords = vec2i(floor(gcoords) + 0.5);
    let localCoords = gcoords - floor(gcoords);

    // Hash the cellCoords to get the refPoint coords
    // Normalize the output to [0,1] instead of [-1,1]
    let refPointCoords = hash(cellCoords) * 0.5 + 0.5;

    // Write the localCoords as color:
    var color = vec4f(localCoords, 0.0, 1.0);

    // Add the border if specified:
    // Compute the "normalized border size" (as borderSize is in pixels):
    let nbsize = params.borderSize / params.gridSize;
    if nbsize > 0.0 {
        let isBorder = localCoords.x < nbsize || localCoords.x > (1.0 - nbsize) || localCoords.y < nbsize || localCoords.y > (1.0 - nbsize);
        let white = vec4f(1.0, 1.0, 1.0, 1.0);
        let black = vec4f(0.0, 0.0, 0.0, 1.0);
        color = select(white, black, isBorder);
    }
    
    // Square Radius in normalized coords space:
    let pointRadiusSqr = params.refPointSize * params.refPointSize;
    
    // Check the distance to the current point
    // and display the refPoint in each cell in red:
    let dir = localCoords - refPointCoords;
    let d2 = dot(dir, dir);
    let red = vec4f(1.0, 0.0, 0.0, 1.0);
    color = select(color, red, d2 < pointRadiusSqr);

    // Write yellow color:
    // let color = vec4f(1.0, 1.0, 0.0, 1.0);

    let layer: u32 = params.origin.z;
    textureStore(outputTex, vec2i(params.origin.xy + id.xy), layer, color);
}
