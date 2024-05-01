@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Kogge-Stone implementation:

// cf. https://github.com/gpuweb/gpuweb/issues/3950
// Hillis-Steele scan
var<workgroup> sdata0: array<vec4u, WG_SIZE>;
var<workgroup> sdata1: array<vec4u, WG_SIZE>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE * 8;
    var x: vec4u = vec4u(
        inputBuffer[gOff + tid],
        inputBuffer[gOff + tid + 1 * WG_SIZE],
        inputBuffer[gOff + tid + 2 * WG_SIZE],
        inputBuffer[gOff + tid + 3 * WG_SIZE]
    );

    var y: vec4u = vec4u(
        inputBuffer[gOff + tid + 4 * WG_SIZE],
        inputBuffer[gOff + tid + 5 * WG_SIZE],
        inputBuffer[gOff + tid + 6 * WG_SIZE],
        inputBuffer[gOff + tid + 7 * WG_SIZE]
    );

    for (var i: u32 = 0; i <= firstTrailingBit(u32(WG_SIZE)); i++) {
        sdata0[tid] = x;
        sdata1[tid] = y;
        workgroupBarrier();
        if tid >= u32(1 << i) {
            x = sdata0[tid - u32(1 << i)] + x;
            y = sdata1[tid - u32(1 << i)] + y;
        }
        workgroupBarrier();
    }

    var val = sdata0[tid];
    outputBuffer[gOff + tid] = val.x;
    outputBuffer[gOff + tid + WG_SIZE] = val.y;
    outputBuffer[gOff + tid + 2 * WG_SIZE] = val.z;
    outputBuffer[gOff + tid + 3 * WG_SIZE] = val.w;
    val = sdata1[tid];
    outputBuffer[gOff + tid + 4 * WG_SIZE] = val.x;
    outputBuffer[gOff + tid + 5 * WG_SIZE] = val.y;
    outputBuffer[gOff + tid + 6 * WG_SIZE] = val.z;
    outputBuffer[gOff + tid + 7 * WG_SIZE] = val.w;
}
