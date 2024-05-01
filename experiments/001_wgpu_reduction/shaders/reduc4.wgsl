// Reduction #4 from nvidia preduction paper:

@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> output: atomic<u32>;

// Create zero-initialized workgroup shared data
const wgsize : u32 = 256;
var<workgroup> sdata: array<u32, wgsize>;

@compute @workgroup_size(wgsize)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {

    // Each thread should read its data:
    var tid: u32 = local_id.x;
    // Scale the groupsize by 2 below to get the overall index:
    var idx: u32 = gid.x * wgsize * 2 + tid;

    // Add 2 elements already:
    // Note: we don't need to check the bounds if using an input array size that can be divided by wgsize
    // sdata[tid] = select(0, inputBuffer[idx], idx < 4194304) + select(0, inputBuffer[idx + wgsize], (idx + wgsize) < 4194304);
    sdata[tid] = inputBuffer[idx] + inputBuffer[idx + wgsize];

    // sync all the threads:
    workgroupBarrier();

    // Do the reduction in shared memory:
    for (var s: u32 = wgsize / 2; s > 0; s >>= 1) {
        if tid < s {
            sdata[tid] += sdata[tid + s];
        }
        workgroupBarrier();
    }


    // Add result from the workgroup to the output storage:
    if tid == 0 {
        atomicAdd(&output, sdata[0]);
    }
}
