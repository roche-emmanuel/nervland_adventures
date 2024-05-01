#include "base/prefixsum"

@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE * 4;
    var val = vec4u(
        inputBuffer[gOff + tid],
        inputBuffer[gOff + tid + WG_SIZE],
        inputBuffer[gOff + tid + 2 * WG_SIZE],
        inputBuffer[gOff + tid + 3 * WG_SIZE],
    );

    val = prefixsum_sklansky_vec4u(tid, val);

    outputBuffer[gOff + tid] = val.x;
    outputBuffer[gOff + tid + WG_SIZE] = val.y;
    outputBuffer[gOff + tid + 2 * WG_SIZE] = val.z;
    outputBuffer[gOff + tid + 3 * WG_SIZE] = val.w;
}
