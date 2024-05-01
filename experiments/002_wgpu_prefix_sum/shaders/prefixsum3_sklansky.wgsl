@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Sklansky scan
var<workgroup> sdata: array<u32, WG_SIZE>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];

#ifdef DEFAULT_IMPL
    workgroupBarrier();


    var offset: u32 = 0;
    for (var j: u32 = 1; j < WG_SIZE; j <<= 1) {
        if (tid & j) != 0 && tid < WG_SIZE {
            sdata[tid] += sdata[((tid >> offset) << offset) - 1];
        }
        workgroupBarrier();
        offset++;
    }

    outputBuffer[gOff + tid] = sdata[tid];
#else 
    // Note: we don't need synchronization between the warps as long as we are 
    // performing the accumulations on chunks of 32 slots. 

    // j=1, offset=0 step:
    if (tid & 1) != 0 {
        sdata[tid] += sdata[tid - 1];
    }
    
    // j=2, offset=1 step:
    if (tid & 2) != 0 {
        sdata[tid] += sdata[ (tid & 254) - 1]; // 254 is 0b11111110
    }

    // j=4, offset=2 step:
    if (tid & 4) != 0 {
        sdata[tid] += sdata[ (tid & 252) - 1]; // 252 is 0b11111100
    }

    // j=8, offset=3 step:
    if (tid & 8) != 0 {
        sdata[tid] += sdata[ (tid & 248) - 1]; // 248 is 0b11111000
    }

    // j=16, offset=4 step:
    if (tid & 16) != 0 {
        sdata[tid] += sdata[ (tid & 240) - 1]; // 240 is 0b11110000
    }
    workgroupBarrier();

    // j=32, offset=5 step:
    if (tid & 32) != 0 {
        sdata[tid] += sdata[ (tid & 224) - 1]; // 224 is 0b11100000
    }
    workgroupBarrier();

    // Read our current value at level Final-2.
    var val = sdata[tid];

    // We are going to add the value at slot 64-1 and slot 3*64-1 to some slots (ie 63 and 191)
    // For stage Final-1
    var s0 = sdata[63];
    var s1 = s0 + sdata[127];
    var s2 = sdata[191];

    // j=64, offset=6 step:
    if (tid & 64) != 0 {
        // sdata[tid] += sdata[ (tid & 192) - 1]; // 192 is 0b11000000
        val += select(s0, s2, tid > 128);
    }

    // We add the value of slot 127 to some slots for the final stage:
    // j=128, offset=7 step:
    if tid >= 128 {
        val += s1;
    }

    outputBuffer[gOff + tid] = val;
#endif
}
