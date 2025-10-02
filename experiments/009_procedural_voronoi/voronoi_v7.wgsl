// cf. https://www.shadertoy.com/view/MslGD8 for reference.
#include "base_utils"

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
@group(0) @binding(1) var<uniform> cam : StdCameraUBO;
@group(0) @binding(2) var outputTex: texture_storage_2d_array<rgba8unorm,write>;

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

fn hash3(p: vec2f) -> vec3f {
    // Simple hash using sin/cos with large multipliers
    // The large numbers help break up patterns
    var r = sin(p.x * 12.9898 + p.y * 78.233) * 43758.5453;
    var g = sin(p.x * 93.9898 + p.y * 67.345) * 27183.1592;
    var b = sin(p.x * 269.5 + p.y * 183.3) * 51829.6737;
    
    // Take fractional part and ensure [0, 1] range
    return vec3f(abs(fract(r)), abs(fract(g)), abs(fract(b)));
}

struct RefPointInfos {
    distance: f32,
    gridPos: vec2f,
};

fn find_closest_ref_point(gridCoords: vec2f) -> RefPointInfos {
    // Start with a very large distance:
    var res = RefPointInfos(10.0, vec2f(0.0));

    // From those coords we can extract the integer part which
    // are the cell coords, and the fractional part which are our pixel
    // local (normalized) coords on the cell:
    let cellCoords = vec2i(floor(gridCoords) + 0.5);
    let localCoords = gridCoords - floor(gridCoords);

    for (var j = -1; j <= 1; j++) {
        for (var i = -1; i <= 1; i++) {
            // Get the coord offset to use from our current cell:
            // This "offset" is what we need to add to our current cell
            // coords to get the sibling cells coords. 
            // Note: this was called "g" in the original implementation. 
            // Note2: we use i32 values directly ourself.
            let cellOffset = vec2i(i, j);

            // hash the sibling cell coords to get its reference point
            // local coords (renormalizing to [0,1] instead of [-1,1])
            // Note: this was called o in the original implementation
            let refPointLocalCoords = hash(cellCoords + cellOffset) * 0.5 + 0.5;
            
            // Now we need to get the vector from our current pixel location
            // to that reference point. We'll use our current cell origin
            // as "frame origin". So in there the sibling ref point coords
            // are: 
            let refCoords = vec2f(cellOffset) + refPointLocalCoords;

            // Now with time animation as in the original Inigo Quilez version:
            let movedRefCoords = vec2f(cellOffset) + (0.5 + 0.5 * sin(cam.time + 6.2831 * refPointLocalCoords));

            // So now the "r" vector is:
            let r = movedRefCoords - localCoords;

            // Compute the squared distance:
            // Note: just called "d" in the original implementation:
            let d2 = dot(r, r);
            
            // And compare to the current best distance:
            // Note: "distance" field actually contains squared distance
            // for now.
            if d2 < res.distance {
                // Replace the best solution:
                res.distance = d2;
                // Get the global grid coords of the ref point:
                res.gridPos = vec2f(cellCoords) + refCoords;
            }
        }
    }

    // Turn squared distance to distance:
    res.distance = sqrt(res.distance);

    return res;
}

@compute @workgroup_size(16, 16, 1)
fn main(@builtin(global_invocation_id) id: vec3u) {

    if id.x > params.textureSize.x || id.y > params.textureSize.y {
        // Nothing to write in this case.
        return;
    }

    // Get our coordinates on the grid:
    let gcoords = vec2f(id.xy) / params.gridSize;

#if 0
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
#else
    // Find the closest ref point for our grid coords:
    let rp = find_closest_ref_point(gcoords);

    // Generate a color based on the selected refPoint grid position:
    // But avoid floating point changes here on a single cell:
    let coords = floor(rp.gridPos * 100.0);
    var rgb = hash3(coords);

    let d = rp.distance;

    // Darken a bit the color the further we get from the
    // reference point (ie. cell center):
    rgb *= clamp(1.0 - 0.4 * d * d, 0.0, 1.0);

    // Display a black dot instead if we are very close
    // to the reference point:
    rgb *= smoothstep(0.08, 0.09, d);

    let color = vec4f(rgb, 1.0);
#endif

    let layer: u32 = params.origin.z;
    textureStore(outputTex, vec2i(params.origin.xy + id.xy), layer, color);
}
