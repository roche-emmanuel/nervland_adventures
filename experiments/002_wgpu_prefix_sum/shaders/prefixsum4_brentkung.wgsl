@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

// cf. https://github.com/b0nes164/GPUPrefixSums?tab=readme-ov-file
// Brnet-Kung scan
var<workgroup> sdata: array<u32, WG_SIZE+1>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    var tid: u32 = local_id.x;

    // load input into shared memory
    var gOff: u32 = gid.x * WG_SIZE;
    sdata[tid] = inputBuffer[gOff + tid];
    workgroupBarrier();

    //Upsweep
    if tid < (WG_SIZE >> 1) {
        sdata[(tid << 1) + 1] += sdata[tid << 1];
    }

    var offset: u32 = 1;
    for (var j: u32 = WG_SIZE >> 2; j > 0; j >>= 1) {
        workgroupBarrier();
        if tid < j {
            sdata[(((tid << 1) + 2) << offset) - 1] += sdata[(((tid << 1) + 1) << offset) - 1];
        }
        offset++;
    }

    //Downsweep
    for (var j: u32 = 1; j <= WG_SIZE; j <<= 1) {
        offset--;
        workgroupBarrier();
        if tid < j {
            sdata[(((tid << 1) + 3) << offset) - 1] += sdata[(((tid << 1) + 2) << offset) - 1];
        }
    }

    // var offset: u32 = 0;
    // for (var j: u32 = 1; j < WG_SIZE; j <<= 1) {
    //     if (tid & j) != 0 && tid < WG_SIZE {
    //         sdata[tid] += sdata[((tid >> offset) << offset) - 1];
    //     }
    //     workgroupBarrier();
    //     offset++;
    // }

    outputBuffer[gOff + tid] = sdata[tid];
}
