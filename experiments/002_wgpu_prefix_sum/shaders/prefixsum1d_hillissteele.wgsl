@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/gpuweb/gpuweb/issues/3950
// Hillis-Steele scan
var<workgroup> xdata: array<u32, WG_SIZE>;
var<workgroup> ydata: array<u32, WG_SIZE>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE * 2;
    var x = inputBuffer[gOff + tid];
    var y = inputBuffer[gOff + tid + WG_SIZE];

    for (var i: u32 = 0; i <= firstTrailingBit(u32(WG_SIZE)); i++) {
        xdata[tid] = x;
        ydata[tid] = y;
        workgroupBarrier();
        if tid >= u32(1 << i) {
            x = xdata[tid - u32(1 << i)] + x;
            y = ydata[tid - u32(1 << i)] + y;
        }
        workgroupBarrier();
    }

    outputBuffer[gOff + tid] = xdata[tid];
    outputBuffer[gOff + tid + WG_SIZE] = ydata[tid];
}
