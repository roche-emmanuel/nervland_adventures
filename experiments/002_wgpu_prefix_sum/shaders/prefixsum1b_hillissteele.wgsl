@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Kogge-Stone implementation:

var<workgroup> sdata: array<u32, WG_SIZE * 2>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];
    workgroupBarrier();

    // Destination buffer index:
    var dstIdx: u32 = WG_SIZE;
    var srcIdx: u32 = 0;
    var temp: u32 = 0;

    for (var i: u32 = 0; i <= firstTrailingBit(u32(WG_SIZE)); i++) {
        var val = sdata[srcIdx + tid];
        val += select(0, sdata[srcIdx + tid - u32(1 << i)], tid >= u32(1 << i));
        sdata[dstIdx + tid] = val;
        workgroupBarrier();
        temp = dstIdx;
        dstIdx = srcIdx;
        srcIdx = temp;
    }

    outputBuffer[gOff + tid] = sdata[srcIdx + tid];
}
