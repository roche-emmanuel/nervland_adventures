// Reduction #7 from nvidia preduction paper:

// Note: in WebGPU, the max number of invocations per workgroup is 256 as opposed to 512 in cuda (?)
// cf. https://gpuweb.github.io/gpuweb/#dom-supported-limits-maxcomputeinvocationsperworkgroup

@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> output: atomic<u32>;

// Create zero-initialized workgroup shared data
var<workgroup> sdata: array<u32, WG_SIZE>;

fn warpReduce(tid: u32) {
#ifdef WG_64
    sdata[tid] += sdata[tid + 32];
#endif
#ifdef WG_32
    sdata[tid] += sdata[tid + 16];
#endif
#ifdef WG_16
    sdata[tid] += sdata[tid + 8];
#endif
#ifdef WG_8
    sdata[tid] += sdata[tid + 4];
#endif
#ifdef WG_4
    sdata[tid] += sdata[tid + 2];
#endif
#ifdef WG_2
    sdata[tid] += sdata[tid + 1];
#endif
}

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {

    // Each thread should read its data:
    var tid: u32 = local_id.x;
    // Scale the groupsize by 2 below to get the overall index:
    var idx: u32 = gid.x * GRID_SIZE + tid;

    // Add multiple elements with gridSize consideration:
    // Note: assuming here we don't need to check the input array bounds.
    var value: u32 = 0;
#ifdef GRID_2
    value += inputBuffer[idx] + inputBuffer[idx + WG_SIZE];
#endif 
#ifdef GRID_4
    value += inputBuffer[idx + 2 * WG_SIZE] + inputBuffer[idx + 3 * WG_SIZE];
#endif 
#ifdef GRID_8
    value += inputBuffer[idx + 4 * WG_SIZE] + inputBuffer[idx + 5 * WG_SIZE];
    value += inputBuffer[idx + 6 * WG_SIZE] + inputBuffer[idx + 7 * WG_SIZE];
#endif 
#ifdef GRID_16
    value += inputBuffer[idx + 8 * WG_SIZE] + inputBuffer[idx + 9 * WG_SIZE];
    value += inputBuffer[idx + 10 * WG_SIZE] + inputBuffer[idx + 11 * WG_SIZE];
    value += inputBuffer[idx + 12 * WG_SIZE] + inputBuffer[idx + 13 * WG_SIZE];
    value += inputBuffer[idx + 14 * WG_SIZE] + inputBuffer[idx + 15 * WG_SIZE];
#endif 
    sdata[tid] = value;

    // sync all the threads:
    workgroupBarrier();

    // Unwrap the for loop:
#ifdef WG_256
    if tid < 128 { sdata[tid] += sdata[tid + 128]; }
    workgroupBarrier();
#endif
#ifdef WG_128
    if tid < 64 { sdata[tid] += sdata[tid + 64]; }
    workgroupBarrier();
#endif

    if tid < 32 {
        warpReduce(tid);
    }

    // Add result from the workgroup to the output storage:
    if tid == 0 {
        atomicAdd(&output, sdata[0]);
    }
}
