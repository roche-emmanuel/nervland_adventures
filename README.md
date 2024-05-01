# NervLand Adventures

Collection of small experiments and coding adventures with my NervLand engine providing some code snippets.

**Important note**: Most of the experimental code provided here either directly or indirectly depends on the "NervLand" engine that I'm currently building on my own, and I'm keeping that project private for the moment, so you **wont be able to just take the code here and build it**. This repository is rather here to share some reference code snippets which you might use to generate some working code in your environment or simply to study the algorithms, etc.

# The experiments

## 001 - WGPU Reduction algorithm

- Folder: **001_wgpu_reduction**
- References:
  - Video Tutorial Part 1: https://www.youtube.com/watch?v=198AoKCB90o
  - Video Tutorial Part 2: https://www.youtube.com/watch?v=R0P9DDRCQ68

This is currently provided as a single C++ unit test file and a collection of wgsl shader files implementing different versions of the reduction algorithm. The unit test will execute the computation with each version and monitor the time taken to compute the bandwidth of the compute shader.

## 002 - WGPU Prefix Sum

- Folder: **002_wgpu_prefix_sum**
- References:
  - Video Tutorial: https://www.youtube.com/watch?v=wD2RezUqaxc

This experiment is a simple continuation from the previous one on the reduction algorithms, this time focusing on prefix-sum computation in WGSL. Again, we are only providing some minimal unit test code in C++ here and the different versions of the compute shaders which may be used as reference for integration into another WebGPU based engine.
