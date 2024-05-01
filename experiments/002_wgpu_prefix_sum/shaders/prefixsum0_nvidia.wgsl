@group(0) @binding(0) var<storage,read> inputBuffer: array<u32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<u32>;

var<workgroup> sdata: array<u32, SDATA_SIZE>;

@compute @workgroup_size(WG_SIZE)
fn main(@builtin(workgroup_id) gid: vec3<u32>, @builtin(local_invocation_id) local_id: vec3<u32>) {
    // Compute the prefix sum:

    var thid: u32 = local_id.x;
    var offset: u32 = 1; 

    // load input into shared memory
    var gOff: u32 = gid.x * GRID_SIZE;

    // Note: below "n" is blockSize or workgroup size.
    var ai: u32 = thid;
    var bi: u32 = thid + (GRID_SIZE / 2);
    var bankOffsetA = CONFLICT_FREE_OFFSET(ai);
    var bankOffsetB = CONFLICT_FREE_OFFSET(bi);

    sdata[ai + bankOffsetA] = inputBuffer[gOff + ai];
    sdata[bi + bankOffsetB] = inputBuffer[gOff + bi];

    // build sum in place up the tree
    for (var d: u32 = GRID_SIZE >> 1; d > 0; d >>= 1) {
        workgroupBarrier();

        if thid < d {
            var ai = offset * (2 * thid + 1)-1;
            var bi = offset * (2 * thid + 2)-1;
            ai += CONFLICT_FREE_OFFSET(ai);
            bi += CONFLICT_FREE_OFFSET(bi);
            sdata[bi] += sdata[ai];
        }

        offset *= 2;
    } 

   // clear the last element  
    if thid == 0 {
        // temp[n - 1 + CONFLICT_FREE_OFFSET(GRID_SIZE - 1)] = 0;
        sdata[GRID_SIZE - 1 + 1] = 0; // offset for gridSize=512 is "1" here.
    } 

   // traverse down tree & build scan 
    for (var d: u32 = 1; d < GRID_SIZE; d *= 2) {
        offset >>= 1;
        workgroupBarrier();

        if thid < d {
            var ai = offset * (2 * thid + 1)-1;
            var bi = offset * (2 * thid + 2)-1;
            ai += CONFLICT_FREE_OFFSET(ai);
            bi += CONFLICT_FREE_OFFSET(bi);

            var t: u32 = sdata[ai];
            sdata[ai] = sdata[bi];
            sdata[bi] += t;
        }
    }

    workgroupBarrier(); 

   // write results to device memory
   // g_odata[2*thid] = temp[2*thid]; 
   // g_odata[2*thid+1] = temp[2*thid+1]; 

    outputBuffer[gOff + ai] = sdata[ai + bankOffsetA];
    outputBuffer[gOff + bi] = sdata[bi + bankOffsetB];
}
