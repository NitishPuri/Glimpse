~~Ray tracer in a weekend.~~
Glimpse.

A physically-based ray tracer that simulates light to reveal scenes with realism and depth.
"Reality is nothing but glimpses of light, shaped by perception and time."

Builds on top https://github.com/NitishPuri/ray-tracer-in-a-weekend


Dependencies

- https://raytracing.github.io
- https://github.com/nothings/stb

Should also use

- https://github.com/skypjack/entt
- https://github.com/g-truc/glm

Reference implementations

- pbrt
- https://github.com/lighttransport/nanort?tab=readme-ov-file

## Roadmap

- Add windowing
  - glfw/imgui
  - something else ?
  - https://github.com/floooh/sokol
  
- Add camera positioning and re-rendering the scene
  - More camera controls, like aperture, exposure etc
- ...
- custom scenes, import/export to json
- setup test scenes
- ...
- improved acceleration structures
  - bvh
- Add effects
  - shadows
  - refraction
  - ...
- ... ??
- global illumination
- ...
- parallelization improvements
  - thread pools ?
  - tiling ?
  - SIMD
  - openMP
-
- ...
- ...??
- ...
- compute shaders for ray tracing
- ...
- vulkan/rtx

