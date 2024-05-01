@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Reduce-then-scan
var<workgroup> sdata: array<u32, WG_SIZE + 1>;
var<workgroup> reducedValues: array<u32, 33>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];
    workgroupBarrier();


    //cant be less than 2
    var spillFactor: u32 = 3;
    var spillSize: u32 = u32(WG_SIZE >> spillFactor); // spillSize will be 32 here if WG_SIZE=256
    
    //Upsweep until desired threshold
    if tid < (WG_SIZE >> 1) {
        sdata[(tid << 1) + 1] += sdata[(tid << 1)];
    }
    workgroupBarrier();

    var offset: u32 = 1;
    var j: u32 = 0;
    for (j = WG_SIZE >> 2; j > spillSize; j >>= 1) {
        if tid < j {
            sdata[(((tid << 1) + 2) << offset) - 1] += sdata[(((tid << 1) + 1) << offset) - 1];
        }
        workgroupBarrier();
        offset++;
    }
    
    //Pass intermediates into secondary buffer
    if tid < j {
        let t: u32 = (((tid << 1) + 2) << offset) - 1;
        reducedValues[tid] = sdata[t] + sdata[(((tid << 1) + 1) << offset) - 1];
        sdata[t] = reducedValues[tid];
    }
    workgroupBarrier();
    
    //Reduce intermediates
    offset = 0;
    for (var j: u32 = 1; j < spillSize; j <<= 1) {
        if (tid & j) != 0 && tid < spillSize {
            reducedValues[tid] += reducedValues[((tid >> offset) << offset) - 1];
        }
        workgroupBarrier();
        offset++;
    }
    
    //Pass in intermediates and downsweep
    offset = spillFactor - 2;
    let t: u32 = (((tid << 1) + 2) << offset) + u32(1 << (offset + 1)) - 1;
    if t < WG_SIZE {
        // InterlockedAdd(sdata[t], g_reduceValues[(t >> spillFactor) - 1]);
        // No need for interlockedadd here in a workgroup (?)
        sdata[t] += reducedValues[(t >> spillFactor) - 1];
    }

    for (var j: u32 = spillSize << 1; j <= WG_SIZE; j <<= 1) {
        workgroupBarrier();
        if tid < j {
            sdata[(((tid << 1) + 3) << offset) - 1] += sdata[(((tid << 1) + 2) << offset) - 1];
        }
        offset--;
    }

    outputBuffer[gOff + tid] = sdata[tid];
}
