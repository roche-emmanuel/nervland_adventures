@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> output: atomic<u32>;

@compute @workgroup_size(256)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    // Accumulate in buffer:
    if id.x < 4194304 {
        atomicAdd(&output, inputBuffer[id.x]);
    }
}
