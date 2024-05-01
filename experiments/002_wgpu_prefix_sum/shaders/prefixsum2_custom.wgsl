@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

var<workgroup> sdata: array<u32, WG_SIZE>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];
    workgroupBarrier();

    // Accumulate the values with a single thread:
    if tid == 0 {
        var count: u32 = 0;
        for (var i: u32 = 0; i < WG_SIZE; i++) {
            count += sdata[i];
            sdata[i] = count;
        }
    }

    workgroupBarrier();

    outputBuffer[gOff + tid] = sdata[tid];
}
