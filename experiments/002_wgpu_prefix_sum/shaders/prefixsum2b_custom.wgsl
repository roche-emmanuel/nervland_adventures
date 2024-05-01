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
    workgroupBarrier();

    // Accumulate the values with a single thread:
    var num: u32 = WG_SIZE / 32;

    if tid < 32 {
        var baseIdx = tid * num;
        var count: u32 = 0;
        for (var i: u32 = 0; i < num; i++) {
            count += sdata[baseIdx + i];
            offsets[tid] = count;
        }

        // Now we perform a reduction on the offsets:
        if tid == 0 {
            var accum: u32 = 0;
            for (var i: u32 = 0; i < 32; i++) {
                count = offsets[i];
                offsets[i] = accum;
                accum += count;
            }
        }

        // Now each thread should write to the output buffer num values:
        count = offsets[tid];
        for (var i: u32 = 0; i < num; i++) {
            count += sdata[baseIdx + i];
            outputBuffer[gOff + baseIdx + i] = count;
        }
    }
}
