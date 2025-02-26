
## Roadmap

This roadmap prioritizes a **progressive learning experience** while keeping it **modular and experimental**.
Also prioritize **algorithmic flexibility** and **infrastructure improvements**, so I can explore different paths at any stage.  

## References and inspirations

- [x] [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
- [x] [Ray Tracing Next Week](https://raytracing.github.io/books/RayTracingNextWeek.html)
- [x] [Ray Tracing the Rest of Your Life](https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html)
- [ ] [pbrt](https://github.com/mmp/pbrt-v4)
- [ ] Smallpaints
- [ ] [nanort](https://github.com/lighttransport/nanort?tab=readme-ov-file)
- [ ] [LuxCore](https://github.com/LuxCoreRender/LuxCore)

## OpenGL Compute shaders
- https://github.com/wsandst/gpu-raytracer/tree/master/src

### Vilkan / RTX integrations
- https://github.com/nvpro-samples/vk_mini_path_tracer?tab=readme-ov-file
- https://github.com/nvpro-samples/vk_raytracing_tutorial_KHR?tab=readme-ov-file
- https://github.com/grigoryoskin/vulkan-compute-ray-tracing/tree/master



### **Refined Roadmap: A Learning-Focused Ray Tracer**  

#### **1. Core Foundation (Basic Ray Tracer + Infrastructure)**
- [x] **Basic Ray Tracing** *(Ray Tracing in One Weekend)*
- [x] **GLFW + ImGui GUI** *(Windowing & Controls)*
- [x] **Refactor into CLI and GUI Apps**
- [x] **Async Ray Tracing in GUI**
- [ ] **Expose All Config Options in GUI** 
  - [x] Scene selection
  - [x] Samples per pixel (SPP)
  - [x] Max depth
  - [x] Camera controls (lookat, vfov, aspect ratio)
  - add mouse controls for camera, something like lux does where rendering restarts on camera move
  - also, make the workload distribution smarter for async ray tracing to get a wave of ray tracing going at once
  - Scene object params like position, material
  - Use SPP=1 to show as close to real time updates as possible while user is manipulating a control, 
    maybe make this togglable ( in cases even spp = 1 might be too much? ), 
- [ ] **Namespace & Modular Code Cleanup**
- Make core into a shared lib
- [ ] **Camera Improvements**
  - Positioning & Re-rendering  
  - Depth of field (Aperture, Focus Distance)
  - Motion blur
  - Exposure & tone mapping
- [ ] use glm for vector stuff.
- [ ] Use something like Conan for package management when trying to add a new package next time.
- [ ] improve project structure. automate, build, test scripts.
- [ ] hook with a unit testing framework 
  - use a header only framework and hook with ctest, much simpler.
  - also, find simple header only lib for image comparison.
  hook with github actions!
- [ ] add unit tests
- Glass, mirrors
- dispersion, rainbows, scattering
- spectral emission/radiance - ??
- add loaders for ply - find ready made solutions ?
- add loaders for lux and pbrt scenes - port their loaders ?
    

#### **2. Scene Management & Experimentation**
- [ ] **Custom Scene Support**
  - Scene import/export as JSON (or other lightweight formats)
  - UI controls to tweak & reload scenes live  
- Mesh rendering
- [ ] **Prebuilt Test Scenes**
  - Cornell Box, Spheres, etc. for comparisons
  - Configurable lighting setups  
- [ ] **Scene Format Loaders**
  - Load scenes from **PBRT, LuxRender, RenderMan**
  - Find open example datasets
  - Benchmark against other renderers!

#### **3. Performance & Algorithmic Enhancements**
- [ ] **Acceleration Structures**
  - [x]  BVH (Bounding Volume Hierarchy)
  - SAH BVH (Surface Area Heuristic)
  - Experiment with alternative structures (KD-Trees, Grids, etc.)  
- Performance profiling
  - Visual Studio, ? what other options?
- [ ] **Parallelization Improvements**
  - Thread pools (C++ standard or custom)  
  - [x] Tiling techniques (Divide image into patches for better caching)  
  - SIMD (AVX, SSE)  
  - OpenMP & Task-based parallelism
    - do i need to setup any infra for this? like for cuda?
- **Raymarching**
    - procedural terrains, SDFs, metaballs for terrain, geometr, clouds, fogs
    - Volumetric effects like fog, haze?
    - Look at perlin noise, marble
- Non photorealistic rendering
    - Toon shading, hatching, stylized effects in ray tracer?
- [ ] **Better Sampling & Noise Reduction**
  - Importance Sampling
  - Adaptive Sampling  
  - Denoising techniques (ML-based, OpenImageDenoise, etc.)  
- [ ] **Lighting & Global Illumination**
  - Soft Shadows  
  - Refraction / Dielectrics  
  - Specular Reflections & Fresnel Effects  
  - Path Tracing / Global Illumination  
  - Multiple Importance Sampling (MIS)  
  - Photon Mapping (if curious)  

#### **4. Compute-Based & Advanced Features**
- [ ] **Compute Shader-Based Ray Tracing**
  - OpenGL Compute Shaders  
  - Experiment with Hybrid CPU/GPU rendering  
- [ ] **Vulkan / RTX Acceleration**
  - Ray Tracing Extensions (RTX, DXR, MetalRT)  
  - BVH Traversal on GPU  
- [ ] **Benchmarking & Profiling**
  - Compare against **PBRT, LuxCore**  
  - Frame Timing & Memory Usage  
  - Hardware-based optimizations (Cache-aware data structures, NUMA-aware memory management)  

---

### **Additional Experimental & Fun Ideas**
- **Reproducibility & Debugging**
  - Implement deterministic random seeds for easy debugging  
  - Visualize BVH structures with a debug mode  
- **Shader-Based Experiments**
  - Port small shaders to GLSL (Raymarching, Procedural Texturing, etc.)  
- **Rendering Non-Photorealistic Effects**
  - Toon Shading & Artistic Rendering  
  - Stylized Reflections / Shadows  
- **AI-Assisted Rendering**
  - ML-Based Denoising  
  - Neural Path Tracing (Check NVIDIA papers)  

