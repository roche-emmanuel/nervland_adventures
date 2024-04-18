// Reduction #1 from nvidia preduction paper:

@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> output: atomic<u32>;

// Create zero-initialized workgroup shared data
const wgsize : u32 = 256;
var<workgroup> sdata: array<u32, wgsize>;

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) id: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {

    // Each thread should read its data:
    var tid: u32 = local_id.x;
    sdata[tid] = select(0, inputBuffer[id.x], id.x < 4194304);

    // sync all the threads:
    workgroupBarrier();

    // Do the reduction in shared memory:
    for (var s: u32 = 1; s < wgsize; s *= 2) {
        if tid % (2 * s) == 0 {
            sdata[tid] += sdata[tid + s];
        }

        workgroupBarrier();
    }

    // Add result from the workgroup to the output storage:
    if tid == 0 {
        atomicAdd(&output, sdata[0]);
    }
}
