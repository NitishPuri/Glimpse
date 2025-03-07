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
- [ ] https://github.com/lighttransport/nanogi
- [ ] [LuxCore](https://github.com/LuxCoreRender/LuxCore)

## OpenGL Compute shaders
- https://github.com/wsandst/gpu-raytracer/tree/master/src

### Vulkan / RTX integrations
- https://github.com/nvpro-samples/vk_mini_path_tracer?tab=readme-ov-file
- https://github.com/nvpro-samples/vk_raytracing_tutorial_KHR?tab=readme-ov-file
- https://github.com/grigoryoskin/vulkan-compute-ray-tracing/tree/master

## Next Steps
- [x] Add uncapped SPP and interactive rendering.
  - Accumulate sample outputs and per sample count in a buffer and update from buffer as fast as possible!. 
  - No double buffering for now.
- [x] Calculate per sample variances while doing this,
- [ ] and show averages to the user, maybe plot graphs!
- [ ] Adaptive sampling
  - Throw more samples where variance is more!
- Add material editors and play with textures!
- [x] Camera Controls

  // TODO:
  //  Add simple denoising using gaussian blur
  //  Adaptive sampling based on pixel variance
  //  (if variance > thres && spp > min_spp) increase spp for this pixel
- [ ] Add methods to visualize bounce paths of a single ray, or a selected region, mapped automatically based on scene geometry.
- [ ] More instrumentation from the ray cast process to analyse what happened, correlating the "fitness" of the resulting sample with the paths it took, and how it can be improved.
- Methods to simplify the process of debugging through a single ray.



# Detailed Roadmap

### **Refined Roadmap: A Learning-Focused Ray Tracer**  

#### **1. Core Foundation (Basic Ray Tracer + Infrastructure)**
- [x] **Basic Ray Tracing** *(Ray Tracing in One Weekend)*
- [x] **GLFW + ImGui GUI** *(Windowing & Controls)*
- [x] **Refactor into CLI and GUI Apps**
- [x] **Async Ray Tracing in GUI**
- [x] **Expose All Config Options in GUI** 
  - [x] Scene selection
  - [x] Samples per pixel (SPP)
  - [x] Max depth
  - [x] Camera controls (lookat, vfov, aspect ratio)
  - [ ] Add mouse controls for camera, something like LuxCore does where rendering restarts on camera move
  - [ ] Make the workload distribution smarter for async ray tracing to get a wave of ray tracing going at once
  - [ ] Scene object params like position, material
  - [ ] Use SPP=1 to show as close to real-time updates as possible while user is manipulating a control, maybe make this togglable (in cases even SPP=1 might be too much)
- [x] Make core into a shared lib
- [ ] **Camera Improvements**
  - [x] Positioning & Re-rendering  
  - [x] Depth of field (Aperture, Focus Distance)
  - [x] Motion blur
- [x] Use glm for vector math.
- [x] Hook with a unit testing framework 
  - [ ] Hook with GitHub Actions!
- [x] Glass, mirrors
- [ ] Dispersion, rainbows, scattering
- [ ] Spectral emission/radiance
- [x] Interactive rendering and uncapped SPP with accumulation buffers

#### **2. Scene Management & Experimentation**
- [ ] **Custom Scene Support**
  - Scene import/export as JSON (or other lightweight formats)
  - UI controls to tweak & reload scenes live  
  - Add loaders for PLY - find ready-made solutions
- [ ] Mesh rendering
- [ ] **Scene Format Loaders**
  - Load scenes from **PBRT, LuxRender, RenderMan**

#### **3. Performance & Algorithmic Enhancements**
- [ ] **Acceleration Structures**
  - [x] BVH (Bounding Volume Hierarchy)
- [ ] Performance profiling
- [ ] **Parallelization Improvements**
  - Thread pools (C++ standard or custom)  
  - [x] Tiling techniques (Divide image into patches for better caching)  
  - SIMD (AVX, SSE)  
- [ ] **Raymarching**
  - Procedural terrains, SDFs, metaballs for terrain, geometry, clouds, fogs
  - Volumetric effects like fog, haze
  - Look at Perlin noise, marble
- [ ] **Non-Photorealistic Rendering**
  - Toon shading, hatching, stylized effects in ray tracer
- [ ] **Better Sampling & Noise Reduction**
  - [x] Importance Sampling
  - Adaptive Sampling  
    - Instead of blindly shooting the same number of rays per pixel, focus more samples on noisy areas.
    - Compute variance per pixel and sample more in high-variance regions.
    - Use a pre-pass (low SPP) to estimate variance, then adaptively refine pixels.
  - Denoising techniques (ML-based, OpenImageDenoise, etc.)  
- [ ] **Lighting & Global Illumination**
  - [x] Soft Shadows  
  - [x] Refraction / Dielectrics  
  - [x] Specular Reflections & Fresnel Effects  
  - [x] Path Tracing / Global Illumination  
  - [x] Multiple Importance Sampling (MIS)  
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

