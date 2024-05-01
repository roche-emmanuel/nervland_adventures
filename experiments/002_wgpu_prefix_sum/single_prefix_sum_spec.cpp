#include <nv_tests_framework.h>

#include <WGPUEngine.h>
#include <numeric>

using namespace nv;
using namespace wgpu;

#define DEBUG_PREFIXSUM 0
#define RUN_ALL 0

static void run_prefix_sum(const char* code, U32 reducFactor,
                           const StringVector& defs, bool inclusive = false,
                           U32 gridSize = 0) {
    // Create a compute pass:
    logNOTE("Running shader {}", code);

    auto* eng = WGPUEngine::instance();
#if DEBUG_PREFIXSUM
    U32 num = 1024;
#else
    U32 num = 4194304; // 2^22
#endif

    RandGen rnd;
    auto in_data = rnd.uniform_int_vector<U32>(num, 0, 4);

    GPUBuffer input(num * sizeof(U32), BufferUsage::Storage, in_data.data());
    GPUBuffer output(num * sizeof(U32),
                     BufferUsage::Storage | BufferUsage::CopySrc);

    // Get the prefixed sums:
    if (gridSize == 0) {
        gridSize = 256 * reducFactor;
    }

    U32 niters = 200;
    Vector<U32> expected(num);
    U32 count = 0;
    if (inclusive) {
        for (U32 i = 0; i < num; ++i) {
            if (i % gridSize == 0) {
                // reset the count:
                count = 0;
            }
            count += in_data[i];
            expected[i] = count;
        }
    } else {
        for (U32 i = 0; i < num; ++i) {
            if (i % gridSize == 0) {
                // reset the count:
                count = 0;
            }
            expected[i] = count;
            count += in_data[i];
        }
    }

    U32 ngrps = ((num + 255) / 256) / reducFactor;
    logNOTE("Using {} workgroups", ngrps);

    auto cpass = create_ref_object<WGPUComputePass>();
    cpass->add_simple_compute({.shaderFile = code,
                               .entries = {input.as_sto(), output.as_rw_sto()},
                               .defs = defs,
                               .dims = {ngrps}});

    auto& bld = eng->build_commands();

    // Dry-run:
    bld.execute_compute_pass(*cpass).submit();

    bld.reset_all();
    // eng->increase_num_timestamps(niters * 2);
    bld.write_timestamp(0);
    for (I32 i = 0; i < niters; ++i) {
        bld.execute_compute_pass(*cpass);
        // bld.execute_simple_compute(
        //     {.shaderFile = code,
        //      .entries = {input.as_sto(), output.as_rw_sto()},
        //      .defs = defs,
        //      .dims = {ngrps}},
        //     2 * i, 2 * i + 1);
    }
    bld.write_timestamp(1);
    bld.submit(false);

    // Should have updated the output buffer now:
    const U32* data2 = (U32*)output.copy_to_staged().read_sync();
    BOOST_REQUIRE(data2 != nullptr);

    // Compare all the value:
    for (U32 i = 0; i < num; ++i) {
#if DEBUG_PREFIXSUM
        logNOTE("Expected/Computed values: {}: {} - {}", i, expected[i],
                data2[i]);
#else
        BOOST_REQUIRE_EQUAL(data2[i], expected[i]);
#endif
    }

    // Read the elapsed time:
    F64 elapsed = eng->get_timestamp_delta_ns(0, 1);
    // F64 elapsed = 0.0;
    // for (I32 i = 0; i < niters; ++i) {
    //     elapsed += eng->get_timestamp_delta_ns(2 * i, 2 * i + 1);
    // }

    // Compute the bandwidth:
    F64 bw = niters * num * sizeof(U32) / (std::pow(1024, 3) * elapsed * 1e-9);
    logNOTE("Compute shader took {} ns, bandwidth: {:.3f} GB/s", elapsed, bw);

    eng->wait_idle();
    // For ref. max RTX 3090 bandwidth is 936.2 GB/s.
}

BOOST_AUTO_TEST_SUITE(single_prefix_sum)

#if RUN_ALL
BOOST_AUTO_TEST_CASE(test_prefixsum0) {
    // cf.
    // https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda
    StringVector defs = {"WG_SIZE=256", "GRID_SIZE=512", "SDATA_SIZE=513",
                         "CONFLICT_FREE_OFFSET(ai)=((ai >> 16) + (ai >> 8))",
                         "CONFLICT_FREE_OFFSET(bi)=((bi >> 16) + (bi >> 8))"};

    run_prefix_sum("tests/prefixsum/prefixsum0_nvidia", 2, defs);
}

BOOST_AUTO_TEST_CASE(test_prefixsum1) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1_hillissteele", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum1b) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1b_hillissteele", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum1c) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1c_hillissteele", 4, defs, true,
                   256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum1d) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1d_hillissteele", 2, defs, true,
                   256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum1e) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1e_hillissteele", 8, defs, true,
                   256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum2) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum2_custom", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum2b) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum2b_custom", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum2c) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum2c_custom", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum3) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum3_sklansky", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum3b) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum3b_sklansky", 4, defs, true, 256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum3c) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum3c_sklansky", 4, defs, true, 256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum4) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum4_brentkung", 1, defs, true);
}

BOOST_AUTO_TEST_CASE(test_prefixsum5) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum5_reducethenscan", 1, defs, true);
}

#else

#if 0
BOOST_AUTO_TEST_CASE(test_prefixsum1e) {
    // cf. https://github.com/gpuweb/gpuweb/issues/3950
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum1e_hillissteele", 8, defs, true,
                   256);
}
#endif

BOOST_AUTO_TEST_CASE(test_prefixsum3c) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum3c_sklansky", 4, defs, true, 256);
}

BOOST_AUTO_TEST_CASE(test_prefixsum3b) {
    StringVector defs = {"WG_SIZE=256"};
    run_prefix_sum("tests/prefixsum/prefixsum3b_sklansky", 4, defs, true, 256);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
