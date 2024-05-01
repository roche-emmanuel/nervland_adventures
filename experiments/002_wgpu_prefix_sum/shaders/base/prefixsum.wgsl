// Prefix-sum computation algorithm

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Sklansky scan
var<workgroup> sdata0: array<vec4u, WG_SIZE>;

fn prefixsum_sklansky_vec4u(tid: u32, inputVal: vec4u) -> vec4u {
    sdata0[tid] = inputVal;
    
    // Note: below we don't need synchronization between the warps as long as we are 
    // performing the accumulations on chunks of 32 slots. 

    // j=1, offset=0 step:
    if (tid & 1) != 0 {
        sdata0[tid] += sdata0[tid - 1];
    }
    
    // j=2, offset=1 step:
    if (tid & 2) != 0 {
        sdata0[tid] += sdata0[ (tid & 254) - 1]; // 254 is 0b11111110
    }

    // j=4, offset=2 step:
    if (tid & 4) != 0 {
        sdata0[tid] += sdata0[ (tid & 252) - 1]; // 252 is 0b11111100
    }

    // j=8, offset=3 step:
    if (tid & 8) != 0 {
        sdata0[tid] += sdata0[ (tid & 248) - 1]; // 248 is 0b11111000
    }

    // j=16, offset=4 step:
    if (tid & 16) != 0 {
        sdata0[tid] += sdata0[ (tid & 240) - 1]; // 240 is 0b11110000
    }
    workgroupBarrier();

    // j=32, offset=5 step:
    if (tid & 32) != 0 {
        sdata0[tid] += sdata0[ (tid & 224) - 1]; // 224 is 0b11100000
    }
    workgroupBarrier();

    // Read our current value at level Final-2.
    var val = sdata0[tid];

    // We are going to add the value at slot 64-1 and slot 3*64-1 to some slots (ie 63 and 191)
    // For stage Final-1
    var s0 = sdata0[63];
    var s1 = s0 + sdata0[127];
    var s2 = sdata0[191];

    // j=64, offset=6 step:
    if (tid & 64) != 0 {
        // sdata0[tid] += sdata0[ (tid & 192) - 1]; // 192 is 0b11000000
        val += select(s0, s2, tid > 128);
    }

    // We add the value of slot 127 to some slots for the final stage:
    // j=128, offset=7 step:
    if tid >= 128 {
        val += s1;
    }

    return val;
}

