# 🌟 NervLand Adventures

Welcome to **NervLand Adventures** — a collection of experimental projects, technical demonstrations, and coding explorations built on the NervLand engine.

> **Note:** Most experiments here depend on my private **NervLand** engine. While you won't be able to build these projects directly, the code serves as valuable reference material for algorithms, implementation patterns, and WebGPU techniques.

---

## 🎯 About This Repository

This repository showcases various graphics programming experiments, rendering techniques, and procedural generation systems. Each experiment is documented with reference code, shader implementations, and accompanying video tutorials to help you understand and adapt these techniques for your own projects.

Whether you're interested in GPU computing, terrain rendering, procedural generation, or WebGPU development, you might find some interesting examples and insights here.

This main page also provide access to the sub-projects online apps that you may freely test and use.

---

## 🚀 Featured Sub-Projects

### [NervForge](https://nervtech.org/nervforge/) — Procedural Tree Generator

A powerful procedural 3D tree generation tool that allows you to create customizable tree models and export them as glTF files.

**Key Features:**

- Multi-level branch generation with customizable density and distribution
- Realistic branch physics with gnarliness and curvature controls
- Dynamic leaf system with multiple textures and tint colors
- Attraction/repulsion systems for natural branch distribution
- Custom texture rendering and upload support
- Export to glTF/GLB format for use in other 3D applications
- Comprehensive configuration system with preset templates (Maple, Willow, etc.)

**Resources:**

- 🌐 [Try it online](https://nervtech.org/nervforge/)

---

#### 📺 Tutorial Series

<details open>
<summary><b>Introduction & Overview</b></summary>

[![NervForge Introduction & Overview](https://img.youtube.com/vi/U93xS8r9G2o/maxresdefault.jpg)](https://www.youtube.com/watch?v=U93xS8r9G2o)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=U93xS8r9G2o)**

Learn how to use NervForge to create customizable 3D tree models right in your browser. This tutorial covers the essential controls including branch level selection, child density adjustment, and the branchless ratio for controlling branch distribution. Discover how to use attraction points and repulsion intensity to create natural-looking branch patterns, apply gnarliness for realistic branch curvature, and customize leaves with various textures and density settings. The video also demonstrates the complete workflow from initial design to exporting your tree as a glTF file for use in other 3D applications.

📝 [Companion Blog Article](https://wiki.nervtech.org/doku.php?id=blog:2026:0307_nervforge_tree_generator) | [Dev.to Article](https://dev.to/the_lone_engineer/nervforge-generate-beautiful-3d-trees-in-your-browser-free-tool-8n2)

</details>

---

<details open>
<summary><b>New Features Update</b></summary>

[![NervForge New Features Update](https://img.youtube.com/vi/ryzXEzazNU0/maxresdefault.jpg)](https://www.youtube.com/watch?v=ryzXEzazNU0)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=ryzXEzazNU0)**

Explore three new features added to NervForge: per-level length scale control for better tree proportions, randomized branch density using the prune density factor for more natural variation, and GLB binary export support for smaller file sizes. This video also provides a behind-the-scenes look at the NervSDK open-source framework, including a detailed walkthrough of the custom glTF implementation found in the `sources/nvk/gltf` folder. See how the glTF classes are structured and learn about the straightforward process of creating glTF files programmatically.

</details>

---

<details open>
<summary><b>Branch Textures & TreeIt Comparison</b></summary>

[![NervForge Branch Textures](https://img.youtube.com/vi/f5rfesCwiF8/maxresdefault.jpg)](https://www.youtube.com/watch?v=f5rfesCwiF8)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=f5rfesCwiF8)**

Branch texture support has arrived! This update introduces 5 bark textures with controllable UV density parameters for both horizontal and vertical tiling. The video explores TreeIt, an impressive tree generation tool that serves as inspiration for future NervForge features including multiple branches at the same location, improved trunk base modeling, and palm tree support. Also covered are important technical fixes for glTF accessor alignment errors and WASM texture loading issues, plus plans for making UV density a per-level setting.

</details>

---

<details open>
<summary><b>Configuration System & External Textures</b></summary>

[![NervForge Configuration System](https://img.youtube.com/vi/Y79s1KmwSTU/maxresdefault.jpg)](https://www.youtube.com/watch?v=Y79s1KmwSTU)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=Y79s1KmwSTU)**

Major workflow improvements with the introduction of a base configuration system for tree presets. This update adds the ability to save and load custom configurations as JSON files, allowing you to build a library of reusable tree templates. New features include per-level UV density parameters for precise texture control on each branch level, per-level attractor positioning for creative branch direction control (like downward-curving branches at one level and upward at another), and external texture storage to keep large files out of the main repository. The configuration system includes reset, save, and load buttons for easy preset management.

</details>

---

<details open>
<summary><b>Debug Build & Performance Fixes</b></summary>

[![NervForge Debug Build & Performance](https://img.youtube.com/vi/hzfgqXZuWcM/maxresdefault.jpg)](https://www.youtube.com/watch?v=hzfgqXZuWcM)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=hzfgqXZuWcM)**

A deep dive into debugging and performance optimization. This technical update covers setting up debug builds with Emscripten source maps for proper C++ debugging in browser dev tools, fixing a critical crash caused by using raw pointers instead of reference pointers in render pass callbacks, and eliminating unnecessary pipeline regeneration when modifying tree settings. The result is dramatically improved performance in the native version, with settings now adjustable in near real-time even for large trees. Also includes the addition of several new leaf textures to expand customization options.

</details>

---

<details open>
<summary><b>Dynamic Textures & Branch Multiplicity</b></summary>

[![NervForge Dynamic Textures & Multiplicity](https://img.youtube.com/vi/OwNVwDqj3bs/maxresdefault.jpg)](https://www.youtube.com/watch?v=OwNVwDqj3bs)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=OwNVwDqj3bs)**

Major usability improvements and new branch control features. This update introduces dynamic texture loading that automatically discovers bark and leaf textures from their respective folders, eliminating manual configuration entries. A scrollbar implementation now handles large texture lists with mouse wheel support. Custom texture upload allows you to load textures directly from your computer and apply them instantly. The highlight is branch multiplicity (node multiplicity), enabling multiple branches to spawn from the same location with controllable position offset for natural distribution. Additional features include child up angle control for precise branch orientation, start spiral angle for controlling initial branch placement around the parent, and UV coordinate flipping for upside-down leaf textures. Includes a new maple tree preset configuration inspired by TreeIt.

</details>

---

<details open>
<summary><b>Willow Trees & Custom Leaf Rendering</b></summary>

[![NervForge Willow Trees & Leaf Rendering](https://img.youtube.com/vi/tS_L_dkh1LA/maxresdefault.jpg)](https://www.youtube.com/watch?v=tS_L_dkh1LA)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=tS_L_dkh1LA)**

Major advancement with custom leaf texture rendering and realistic leaf deformation. The new render tab allows you to generate custom leaf textures by capturing any part of your tree with adjustable field of view, resolution, and aspect ratio controls. Leaf rendering now supports subdivision with U and V resolution parameters, enabling realistic bending and drooping effects controlled by leaf curve, attraction direction, and power/scale parameters. This update includes a complete walkthrough of building a willow tree configuration from scratch, demonstrating the full workflow from creating a custom willow leaf texture to assembling a realistic drooping willow tree. Also includes a critical memory leak fix preventing mesh data accumulation, and improved configuration file management with automatic folder-based config discovery.

</details>

---

<details open>
<summary><b>Tint Colors for Bark & Leaves</b></summary>

[![NervForge Tint Colors](https://img.youtube.com/vi/kVeFY2sItlk/maxresdefault.jpg)](https://www.youtube.com/watch?v=kVeFY2sItlk)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=kVeFY2sItlk)**

Simple but powerful color customization feature. This update adds tint color support for both bark and leaves, allowing you to apply color multipliers to textures for creative effects. The bark tint checkbox on the tree designer panel lets you colorize the entire trunk and branches, while the leaf tint checkbox on the leaves tab provides separate color control for foliage. Both features can be toggled on/off to optimize mesh size when not needed, and tint colors are properly preserved during glTF export, maintaining texture details underneath the color overlay.

</details>

---

<details open>
<summary><b>Building Initial Bush Config</b></summary>

[![NervForge Building Initial Bush Config](https://img.youtube.com/vi/sP6pKIvyMi8/maxresdefault.jpg)](https://www.youtube.com/watch?v=sP6pKIvyMi8)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=sP6pKIvyMi8)**

Taking NervForge from computer-generated looking bushes to natural, realistic vegetation through several targeted fixes and optimizations. Covers moving from single-origin spawning to disc/square spawn areas for natural root distribution, a font architecture refactor to eliminate widget-to-canvas dependencies, and a fix for invisible child widgets caused by missing parent reference propagation. The centerpiece is the self-repulsion system — preventing branches from intersecting at the root level — including a full performance journey from 16ms to 364ms back down to 58ms through deferred repulsion levels, spatial grid optimization, and weighted center-of-mass approximation per grid cell. Also includes global uniform/non-uniform scale controls and proportional leaf width curves based on leaf length.

📝 [Companion Blog Article](https://wiki.nervtech.org/doku.php?id=blog:2026:0314_initial_bush_config)

</details>

---

<details open>
<summary><b>Dynamic Texture Loading with Async Promise System</b></summary>

[![NervForge Dynamic Texture Loading](https://img.youtube.com/vi/NkUmhi6WMio/maxresdefault.jpg)](https://www.youtube.com/watch?v=NkUmhi6WMio)

**[🎦 Watch on YouTube](https://www.youtube.com/watch?v=NkUmhi6WMio)**

A focused update introducing on-demand texture loading to NervForge, powered by a new promise-based async system in NervSDK. Textures are now listed in a remote resource manifest and downloaded only when selected, replacing the previous approach of bundling everything upfront. While a texture downloads, a checkerboard placeholder material is shown in its place; once the download completes, the resolution callback executes on the main thread and triggers a recursive `setLeafTexture` call to finalize the material swap. Downloaded textures are cached locally so they persist across sessions without redundant re-fetches. Also includes a fix for black artifacts appearing at the planet horizon level.

</details>

---

### [TerrainView7](https://nervtech.org/terrainview7) — Atmospheric Scattering

![TerrainView7](images/terrainview7_preview.png)

An advanced terrain rendering demo featuring precomputed atmospheric scattering for realistic sky and lighting effects.

**Key Features:**

- Precomputed atmospheric scattering implementation
- Realistic sky rendering with day/night cycles
- Advanced terrain LOD system
- WebGPU-powered rendering pipeline

**Resources:**

- 🌐 [Live Demo](https://nervtech.org/terrainview7) (WebGPU required)
- 📺 [Overview Video](https://www.youtube.com/watch?v=85-VGX808xA)

---

### [TerrainView8](https://nervtech.org/terrainview8) — Realistic Ocean Rendering

![TerrainView8](images/terrainview8_preview.png)

Real-time ocean rendering with seamless transitions from geometry to BRDF, featuring dynamic waves and foam.

**Key Features:**

- Based on Eric Bruneton's ocean lighting research paper
- Full WebGPU compute shader implementation for spectrum/FFT computation
- Elfouhaily wave spectrum model for realistic wave patterns
- Dynamic foam rendering at wave peaks and shores
- Beer-Lambert Law for underwater bedrock visualization
- Controllable wind direction and wave parameters

**Resources:**

- 🌐 [Live Demo](https://nervtech.org/terrainview8) (WebGPU required)
- 📺 [Feature Overview](https://www.youtube.com/watch?v=uUomhFu364I)
- 📄 [Research Paper](https://hal.science/inria-00443630)

---

## 🧪 Technical Experiments

### 001 — WGPU Reduction Algorithm

GPU-based parallel reduction implementations with performance benchmarking.

- Multiple shader variants exploring different optimization strategies
- Bandwidth monitoring and performance analysis
- 📺 [Tutorial Part 1](https://www.youtube.com/watch?v=198AoKCB90o) | [Part 2](https://www.youtube.com/watch?v=R0P9DDRCQ68)
- 📁 `experiments/001_wgpu_reduction/`

---

### 002 — WGPU Prefix Sum

Parallel prefix sum (scan) algorithm implementation in WGSL compute shaders.

- Building on reduction algorithm concepts
- Multiple optimization variants
- 📺 [Tutorial Video](https://www.youtube.com/watch?v=wD2RezUqaxc)
- 📁 `experiments/002_wgpu_prefix_sum/`

---

### 003 — WGPU Native/WASM App

Cross-platform WebGPU application framework supporting both native and web deployment.

- **Self-contained** — No NervLand dependencies, can be built standalone
- Emscripten toolchain for WASM compilation
- Single codebase for native and browser targets
- 📁 `experiments/003_wgpu_native_wasm_app/`

---

### 004 — FFmpeg Video Playback in Native WGPU

![FFmpeg Playback](images/ffmpeg_video_in_nervland.png)

Hardware-accelerated video playback using DirectX 11 and Dawn's Shared Texture API.

- DirectX 11 video stream generation
- WebGPU texture sharing and rendering
- Full hardware acceleration pipeline
- **Windows only** implementation
- 📺 [Implementation Tutorial](https://www.youtube.com/watch?v=P1jxvLm6SwE)
- 📁 `experiments/008_ffmpeg_video_playback/`

---

### 005 — Procedural Voronoi Texture Generation

![Procedural Voronoi](images/voronoi_quad_in_nervland.png)

GPU-based procedural Voronoi diagram generation with progressive complexity.

- Beginner-friendly iterative implementation
- Multiple shader versions showing progression
- Complete compute shader workflow
- 📺 [Video Tutorial](https://www.youtube.com/watch?v=kNgqw7HKzmg)
- 📝 [Written Tutorial](https://dev.to/the_lone_engineer/tutorial-procedural-voronoi-texture-generation-in-wgpu-1b3k)
- 📁 `experiments/005_procedural_voronoi/`

---

## 🛠️ Technologies

- **WebGPU** — Modern GPU API for graphics and compute
- **WGSL** — WebGPU Shading Language
- **C++** — Core engine implementation
- **Emscripten** — WASM compilation for web deployment
- **glTF** — 3D model export format

---

## 🔗 Related Projects

- **NervSDK** — Open-source framework with base components ([GitHub](https://github.com/roche-emmanuel/NervSDK))
  - Custom glTF implementation (`sources/nvk/gltf`)
  - Cross-platform base utilities
  - Built with NervProj mechanism
- **NervProj** — Open-source project management framework ([GitHub](https://github.com/roche-emmanuel/NervProj))

---

## 📬 Feedback & Questions

Have questions about implementation details? Feel free to open an issue or reach out through the video comments or ask on the [💬 project sub-reddit](https://www.reddit.com/r/Project_NervLand/)

---

## ⚖️ License

Reference code provided for educational purposes. Everything provided here is under MT license.

---

<div align="center">

**[📺 YouTube Channel](https://www.youtube.com/channel/UCqyXrUfNuzIW5Pn1H9CIMSg/)** | **[💬 Community](https://www.reddit.com/r/Project_NervLand/)**

_Building the future of graphics programming, one experiment at a time._

</div>
