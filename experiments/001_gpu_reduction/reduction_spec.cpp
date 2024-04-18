#include <nv_tests_framework.h>

#include <WGPUEngine.h>
#include <numeric>

using namespace nv;
using namespace wgpu;

static void run_reduction(const char* code, U32 reducFactor = 1,
                          StringVector* defs = nullptr) {
    // Create a compute pass:
    auto* eng = WGPUEngine::instance();

    // U32 num = 1e6;
    U32 num = 4194304; // 2^22

    RandGen rnd;
    auto in_data = rnd.uniform_int_vector<U32>(num, 0, 4);

    GPUBuffer input(num * sizeof(U32), BufferUsage::Storage, in_data.data());
    GPUBuffer output(1 * sizeof(U32),
                     BufferUsage::Storage | BufferUsage::CopySrc);

    // Get the accumulated value:
    U32 niters = 200;
    // Note: we need to account for the dry-run below:
    U32 total =
        std::accumulate(in_data.begin(), in_data.end(), 0U) * (niters + 1);
    U32 ngrps = ((num + 255) / 256) / reducFactor;
    logNOTE("Using {} workgroups", ngrps);

    auto cpass = create_ref_object<WGPUComputePass>();
    cpass->add_simple_compute({.shaderFile = code,
                               .entries = {input.as_sto(), output.as_rw_sto()},
                               .defs = defs != nullptr ? *defs : StringVector{},
                               .dims = {ngrps}});

    auto& bld = eng->build_commands();

    // Dry-run:
    bld.execute_compute_pass(*cpass);
    bld.submit();

    bld.reset_all();
    // eng->increase_num_timestamps(niters * 2);
    bld.write_timestamp(0);
    for (I32 i = 0; i < niters; ++i) {
        bld.execute_compute_pass(*cpass);
        // bld.execute_simple_compute(
        //     {.shaderFile = code,
        //      .entries = {input.as_sto(), output.as_rw_sto()},
        //      .defs = defs != nullptr ? *defs : StringVector{},
        //      .dims = {ngrps}},
        //     2 * i, 2 * i + 1);
    }
    bld.write_timestamp(1);
    bld.submit(false);

    // Should have updated the output buffer now:
    const U32* data2 = (U32*)output.copy_to_staged().read_sync();

    logNOTE("Expected reduction total: {}", total);
    BOOST_REQUIRE(data2 != nullptr);
    BOOST_CHECK_EQUAL(*data2, total);

    // Read the elapsed time:
    F64 elapsed = eng->get_timestamp_delta_ns(0, 1);
    // F64 elapsed = 0.0;
    // for (I32 i = 0; i < niters; ++i) {
    //     elapsed += eng->get_timestamp_delta_ns(2 * i, 2 * i + 1);
    // }

    // Compute the bandwidth:
    F64 bw = niters * num * sizeof(U32) / (std::pow(1024, 3) * elapsed * 1e-9);
    logNOTE("Compute shader took {} ns, bandwidth: {:.3f} GB/s", elapsed, bw);

    // For ref. max RTX 3090 bandwidth is 936.2 GB/s.
}

BOOST_AUTO_TEST_SUITE(reduction)

BOOST_AUTO_TEST_CASE(test_reduc0) { run_reduction("tests/reduction/reduc0"); }
BOOST_AUTO_TEST_CASE(test_reduc1) { run_reduction("tests/reduction/reduc1"); }
BOOST_AUTO_TEST_CASE(test_reduc3) { run_reduction("tests/reduction/reduc3"); }
BOOST_AUTO_TEST_CASE(test_reduc4) {
    run_reduction("tests/reduction/reduc4", 2);
}
BOOST_AUTO_TEST_CASE(test_reduc5) {
    run_reduction("tests/reduction/reduc5", 2);
}
BOOST_AUTO_TEST_CASE(test_reduc6) {
    StringVector defs = {
        "WG_SIZE=256", "WG_256", "WG_128", "WG_64", "WG_32",
        "WG_16",       "WG_8",   "WG_4",   "WG_2",
    };
    run_reduction("tests/reduction/reduc6", 2, &defs);
}
BOOST_AUTO_TEST_CASE(test_reduc7) {
    // StringVector defs = {"WG_SIZE=256", "WG_256",         "WG_128", "WG_64",
    //                      "WG_32",       "WG_16",          "WG_8",   "WG_4",
    //                      "WG_2",        "GRID_SIZE=2048", "GRID_2", "GRID_4",
    //                      "GRID_8"};
    // run_reduction("tests/reduction/reduc7", 8, &defs);
    StringVector defs = {"WG_SIZE=256", "WG_256",         "WG_128", "WG_64",
                         "WG_32",       "WG_16",          "WG_8",   "WG_4",
                         "WG_2",        "GRID_SIZE=4096", "GRID_2", "GRID_4",
                         "GRID_8",      "GRID_16"};
    run_reduction("tests/reduction/reduc7", 16, &defs);
}

BOOST_AUTO_TEST_SUITE_END()
