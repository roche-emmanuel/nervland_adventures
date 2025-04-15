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

## 003 - WGPU Native/WASM app

- Folder: **003_wgpu_native_wasm_app**
- References:
  - Video Tutorial: **to be released**

In this folder you will find the 4 test applications I built to reach a simple code base which could now be used to build a WebGPU base app both as a native application or as a WASM application to run in your browser.

Those test apps do not depend on the NervLand libraries so it should be possible to build them with the provided code without too much trouble (assuming you have a proper emscripten toolchain available for the build process)

## 004 - TerrainView5

**TODO**

## 005 - TerrainView6

**TODO**

## 006 - TerrainView7

![TerrainView7](006_terrainview7/terrainview7_preview.png)

The TerrainView7 tech demo app introduces support for the Precomputed Atmospheric Scattering on the terrain rendering layer.

- If your browser supports WebGPU, you can give this demo a try at: https://nervtech.org/terrainview7
- And if you want a quick overview on the new features/changes I introduced in this release compared to the **TerrainView6**, you can check the companion video at: https://youtu.be/85-VGX808xA
