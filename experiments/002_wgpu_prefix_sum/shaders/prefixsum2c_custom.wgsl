@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

var<workgroup> sdata: array<u32, WG_SIZE>;
var<workgroup> offsets: array<u32, 32>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];

    // We don't have to wait just yet: we can instead tranform the data that we have already loaded for this warp:
    // We have 4 pack of 8 slots on which we could perform a prefix computation:
    var warpIdx = tid / 32;
    
    // get the thread id in the warp:
    // This will be between in range [0, 31]:
    var twIdx = tid - 32 * warpIdx;

    // if the thread index in the warp is less that 4 we use that thread
    // to perform a prefix sum:
    if twIdx < 4 {
        // Get the pack base index:
        var baseIdx = warpIdx * 32 + twIdx * 8;
        var count: u32 = 0;
        for (var i: u32 = 0; i < 8; i++) {
            count += sdata[baseIdx + i];
            sdata[baseIdx + i] = count;
        }

        // Now also write the final count value for that pack as offset:
        offsets[warpIdx * 4 + twIdx] = count;
    }

    // Synchronize all threads:
    workgroupBarrier();

    // accumulate the offsets:
    if tid == 0 {
        var accum: u32 = 0;

        for (var i = 0; i < 32; i++) {
            var val = offsets[i];
            offsets[i] = accum;
            accum += val;
        }
    }

    workgroupBarrier();

    // Now each thread should write the value from shared mem plus the offset of that pack:
    outputBuffer[gOff + tid] = sdata[tid] + offsets[tid / 8];
}
