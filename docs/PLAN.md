# Glimpse — Learning Path

Glimpse is the CPU path tracer in which each ray tracing technique is learned and measured first. Later
the same integrator is ported to the GPU, and the CPU renders become the reference images. It is the
ray tracing companion to Glint (`/mnt/e/tree/graphics/glint`), which follows GPU Gems with GL/VK.

Agreed 2026-10-10. The Obsidian vault mirrors this plan:
`1_TOPICS/Tech/Tech Projects/Glimpse/Glimpse - Learning Path.md`, plus one `Glimpse - Stage N - <name>.md` per
stage with the reading list and a Learnings section. The pbrt reading checklist is
`1_TOPICS/Tech/Computer Graphics/Ray Tracing/PBRT v4 - Reading Index.md`.

Work top to bottom. Tick boxes as work lands, and commit each tick together with its work.
`ROADMAP.md` is the older wishlist, kept for history. Its open items are folded in below.

---

## How the order was chosen

1. **Three parts.**
   - **A, measure then reduce noise:** tooling that turns "less noise" into a number, then the core
     variance-reduction techniques on the existing simple scenes.
   - **B, content and advanced transport:** meshes, scene files, lights, volumes and the harder
     integrators, which need real scenes.
   - **C, GPU:** port the integrator, and judge every GPU image against the CPU reference.
2. **One new idea per technique.** A technique adds one rendering idea. When it needs a refactor (for
   example recursive → iterative), that refactor is part of the technique and is written up in its note.
3. **The code changes only when a stage needs it.** There is no up-front restructure. The RTIOW-shaped core
   (recursive `ray_color`, `material::scatter`, `shared_ptr`, `double`) changes only when a technique
   needs it to.
4. **Old methods stay selectable.** When a technique replaces a method, the old one stays behind a flag
   (`--integrator`, `--sampler`, ...). That keeps equal-time comparisons and viewer toggles possible.
5. **Reading spine.** For each technique:
   - **pbrt-v4** (online at <https://pbr-book.org/4ed/>, source at
     `/mnt/e/tree/graphics/ray_tracing/pbrt/pbrt-v4`) gives the theory and the order.
   - **Ray Tracing Gems I/II** articles (open access) are the "gems" to replicate.
   - pbrt's 4th edition dropped BDPT and photon mapping. Those come from the
     [3rd edition, ch16](https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods) and the local
     `pbrt-v3` source.
   - Optional video companion:
     [TU Wien Rendering](https://www.cg.tuwien.ac.at/courses/Rendering/VU.SS2019.html).
6. **Hardware.** The RTX 3060 has worked since 2026-10-10 (laptop in `nvidia` graphics mode; see Glint's notes).
   Nothing is gated any more.

## Definition of done (per technique)

- [ ] It renders its test scene from the CLI to a **float image** (PFM).
- [ ] The scene has a **reference**: the same scene at very high SPP, rendered once and cached under `refs/`
      (gitignored).
- [ ] The comparison tool reports **RMSE and relMSE** against the reference, plus an **equal-time comparison**
      with the method it improves on. pbrt-v4's `imgtool diff` is an independent cross-check.
- [ ] `docs/techniques/NN_name.md` is written, following the template below.
- [ ] Unit tests still pass. New math (warps, pdfs, BSDF reciprocity and energy) gets a unit test.
- [ ] Learnings are added to the vault stage note, and its reading items are ticked.

Pace: about one technique per session, one commit per technique. Review at the end of each stage and
reorder, drop or swap whatever turned out to be (un)interesting.

### Technique note template — `docs/techniques/NN_name.md`

```markdown
# NN · name
**Stage:** N · **Status:** ✅ YYYY-MM-DD
## Read
- pbrt-v4 §x.y — <link> · source: `src/pbrt/...`
- RTG I/II ch N — <link>
## Idea          (one paragraph of theory, in my own words)
## What changed  (files, refactors and why)
## Numbers       (scene, spp/time, RMSE/relMSE vs reference, equal-time vs previous method, image)
## Gotchas
## Repro         (exact CLI commands)
```

---

## Stage 0 — Foundation on Linux (infrastructure)

- [x] Fix the GCC link error in `Glimpse_tests`. `ut::cfg<ut::override>` is specialized after it has
      already been used in other files. Then get the suite passing (or document the known failures).
- [x] Make the CLI flags real: `--scene <name|index> --spp --width --depth --seed --out`. Use
      `--list-scenes`. Remove the hardcoded Cornell light from `main.cpp` and let scenes declare their own
      `lights`.
- [x] `Glimpse_gui` builds on Linux: GLFW from FetchContent or the system (as Glint did), `OpenGL::GL`,
      and the ImGui/glad sources from `ext/ext.7z` or FetchContent.
- [x] Clean up the build: one `CMakePresets.json` preset for Linux/Ninja, `compile_commands.json`, and
      `.gitattributes` for line endings (`README.md`/`ROADMAP.md` are CRLF today, new files are LF). Add
      `refs/` and `results/` to `.gitignore` if they are missing.
- [x] ~~Optional: re-enable a Linux CI job~~ skipped (2026-10-11): local `ctest` is enough for now.
- [ ] **Structure review** (added 2026-10-11). Glimpse was built by hand in 2025 without today's goals, while
      Glint was started fresh with them. Before Stage 1 builds on the current layout, check how well it fits:
  - **Folders and file names:** do `src/core`, `hittables/`, the header-heavy files and the scene registry
    scale to 29 techniques? Where do per-technique code, notes, test scenes and references live? (Glint has
    `techniques/NN_name/` with code and `NOTES.md`, plus `tools/`, `docs/`, `cmake/` and `external/`.)
  - **Conventions:** naming (RTIOW snake_case vs pbrt/Glint style), namespaces, one class per file, headers vs
    `.cpp`, `ext/` vs FetchContent, where logs and results go.
  - **Readability:** the goal is literate, pbrt-like code that a reader learns from: comments that explain
    the math and the *why*, consistent terms (radiance, pdf, measure), and no dead code. The `#if 0` files,
    commented-out experiments and stale TODOs get resolved (revived, moved into the plan, or deleted).
  - Deliverable: `docs/structure.md` with the findings and a proposed layout, reviewed by the owner. Approved
    changes are applied as mechanical, behaviour-preserving moves and renames (the tests must still pass)
    before Stage 1. Anything that changes behaviour stays with the technique that needs it.

**Done when:** a clean clone configures and builds with one command on Linux, the tests pass, the viewer
runs, and `Glimpse_cli --scene cornell_box --spp 64 --out x.png` writes an image.

---

## Part A — Measure, then reduce noise

### Stage 1 — Measuring

- [ ] **01 `float_output`**
  - `Image` keeps linear float data, and `--out x.pfm` writes it.
  - PNG/JPG output uses the proper sRGB transfer function with exposure, replacing the `sqrt` gamma.
  - `--reference` renders at very high SPP into `refs/<scene>.pfm`.
- [ ] **02 `error_metrics`**
  - A comparison tool (`Glimpse_cli compare a.pfm b.pfm`) prints RMSE and relMSE and can write a
    false-colour error image.
  - A convergence mode writes error against SPP and against time to CSV.
  - The viewer plots the live error against the reference with ImPlot.
- [ ] **03 `deterministic_rng`**
  - Each (pixel, sample index) gets its own seed (hashed), so images are the same for any thread count.
  - Threads take tiles from a queue instead of fixed row ranges.
  - This fixes the flaky e2e tests: compare floats against the reference with a relMSE threshold.

Read: pbrt §2.1–2.2, §4.6, §5.4, §8.3, §B.5; RTG1 ch15 *On the Importance of Sampling*; RTG1 ch10 *A Simple
Load-Balancing Scheme* (03).

### Stage 2 — Sampling

- [ ] **04 `samplers`:** a `Sampler` interface with independent, stratified, Halton and Sobol (Owen-scrambled)
      implementations. It replaces `random_double()` in the camera and at each bounce. Equal-SPP error
      comparison.
- [ ] **05 `adaptive_sampling`:** turn on the Welford variance in `Film`. Pixels get a minimum SPP, then a
      stop criterion based on variance. Heatmaps of variance and SPP in the viewer. Equal-time comparison
      against uniform SPP.
- [ ] **06 `pixel_filters`:** box, tent, Gaussian and Mitchell reconstruction filters, using filter
      importance sampling (pbrt-v4 style).

Read: pbrt §2.3–2.4, §8.1–8.8, §A.4–A.5; RTG1 ch16 *Sampling Transformations Zoo*.

### Stage 3 — Light transport core

- [ ] **07 `iterative_path`**
  - The recursive `ray_color` becomes a loop with a running throughput and Russian roulette.
  - The new loop must produce the same image as the old one, within noise, against the reference.
- [ ] **08 `nee`**
  - Next-event estimation: sample a light, cast a shadow ray, and avoid counting emission twice.
  - Robust ray offsets (RTG1 ch6) replace the `0.001` epsilon.
- [ ] **09 `mis`**
  - The power heuristic combines BSDF sampling and light sampling.
  - Compare it with the RTIOW mixture-pdf method (`--integrator naive|mixture|nee|mis`).
  - Fireflies and clamping (RTG1 ch17) as a side note.
- [ ] **10 `path_inspector`**
  - Click a pixel in the viewer to record its path vertices, then draw them over the image and list the
    pdfs, weights and contributions at each bounce.
  - Written fresh. The dormant 2025 attempt (`ray_visualizer` / `ray_analysis` / `ray_debug`) was deleted in the
    Stage 0 structure review; git history has it.

Read: pbrt §13.1–13.4, §2.2 (MIS), §6.8 (rounding error); RTG1 ch6 *Avoiding Self-Intersection*, ch17
*Ignoring the Inconvenient*; RTG2 ch14 *The Reference Path Tracer*, ch20 *Multiple Importance Sampling 101*.

### Stage 4 — Materials

- [ ] **11 `bsdf_interface`**
  - A shading frame plus a `BSDF` with `f` / `sample_f` / `pdf`, replacing `material::scatter`.
  - Lambertian, mirror and smooth dielectric are ported to it, with unit tests for reciprocity and
    energy conservation.
- [ ] **12 `microfacet_conductor`:** Trowbridge-Reitz (GGX) with visible-normal sampling and the
      complex-IOR Fresnel equations. A roughness sweep scene.
- [ ] **13 `rough_dielectric_coated`:** rough glass, and a coated diffuse material (pbrt's layered BxDF).

Read: pbrt §9.1–9.7, §10.5, §14.3; RTG1 ch12 *Bump Terminator*; RTG1 ch16.6.5 (GGX sampling);
[Heitz 2018, Sampling the GGX Distribution of Visible Normals](https://jcgt.org/published/0007/04/01/).

---

## Part B — Content and advanced transport

### Stage 5 — Geometry and scenes

- [ ] **14 `triangle_meshes`:** OBJ/PLY loading, watertight ray/triangle intersection, interpolated normals
      and UVs.
- [ ] **15 `sah_bvh`:** a BVH built with the surface-area heuristic and flattened into an array (pbrt's
      `LinearBVHNode`). Benchmarked against the current midpoint BVH (rays/s, build time).
- [ ] **16 `pbrt_scene_loader`**
  - Loads a subset of the pbrt-v4 scene format (shapes, the materials we have, area and infinite lights,
    camera).
  - Scenes from `/mnt/e/tree/graphics/ray_tracing/pbrt/pbrt_scenes/pbrt-v4-scenes` become test scenes.
  - Cross-check against a local pbrt-v4 render.

Read: pbrt §6.1, §6.5, §6.8, §7.1–7.3, Appendix C; RTG1 ch7 *Precision Improvements for Ray/Sphere
Intersection*; [pbrt-v4 file format](https://pbrt.org/fileformat-v4).

### Stage 6 — Lights and colour

- [ ] **17 `envmap_importance`:** an HDR environment light (EXR/HDR) sampled with a piecewise-constant 2D
      distribution, combined with MIS.
- [ ] **18 `many_lights`:** choose lights by power with the alias method, then with a light BVH. Compared on
      a scene with hundreds of emitters.
- [ ] **19 `spectral`:** hero-wavelength spectral rendering (pbrt's `SampledWavelengths`), RGB-to-spectrum
      conversion, and dispersion in glass (prisms and rainbows from the roadmap).

Read: pbrt §12.1–12.6, §4.1–4.6, §A.1, §A.5; RTG1 ch16.4.2 (sampling a 2D texture), ch18 *Importance
Sampling of Many Lights on the GPU*; RTG2 ch21 *The Alias Method*.

### Stage 7 — Volumes

- [ ] **20 `media`:** a medium interface, homogeneous media, the Henyey-Greenstein phase function, and
      delta and ratio tracking. The RTIOW `constant_medium` becomes one case of it. Volumetric path
      tracing with NEE.
- [ ] **21 `hetero_volumes`:** grid and procedural (Perlin) density. Clouds and smoke. NanoVDB is optional.

Read: pbrt §11.1–11.4, §14.1–14.2; RTG1 ch16.7 (volume sampling), ch28 *Ray Tracing Inhomogeneous Volumes*,
ch11 *Nested Volumes* (optional).

### Stage 8 — Advanced integrators

- [ ] **22 `bdpt`:** bidirectional path tracing with MIS over connection strategies. Caustic-heavy test scenes.
- [ ] **23 `sppm`:** stochastic progressive photon mapping. Caustics compared with BDPT and the path tracer.
- [ ] **24 `restir_di`:** resampled importance sampling, then reservoirs (spatial reuse on the CPU, as
      groundwork for the GPU).
- [ ] **25 `denoise`:** Intel Open Image Denoise with albedo and normal buffers. Measure the error the
      denoiser adds against the reference, not only how it looks.

Read: pbrt-v3 §16.1–16.3 (+ `pbrt-v3/src/integrators/{bdpt,sppm}.cpp`); pbrt-v4 §A.2 *Reservoir Sampling*;
RTG1 ch24 *Real-Time Global Illumination with Photon Mapping*; RTG2 ch22 *Weighted Reservoir Sampling*, ch23
*Rendering Many Lights with Grid-Based Reservoirs*; [OIDN](https://www.openimagedenoise.org).

---

## Part C — GPU, with the CPU renders as references

### Stage 9 — GPU

- [ ] **26 `data_oriented_core`**
  - Flat arrays, `float`, no virtual calls (tagged unions / index-based materials), BVH nodes uploadable
    as-is.
  - The CPU output must stay identical, with the same relMSE against the existing references.
- [ ] **27 `compute_megakernel`:** a single compute-shader path tracer that renders the same scenes, judged
      by relMSE against the CPU references. **API decided at stage start** (leaning Vulkan compute, since 29
      needs Vulkan).
- [ ] **28 `wavefront`:** split the megakernel into per-stage queues (generate, intersect, shade, shadow),
      following pbrt ch15. Compare throughput with 27.
- [ ] **29 `vulkan_rt`:** hardware ray tracing on the RTX 3060 (acceleration structures, ray query or the RT
      pipeline) behind the same integrator. Glint Stage 15 covers RT shadows and AO in a rasterizer; this
      is the full path tracer.

Read: pbrt §15.1–15.3 (+ `pbrt-v4/src/pbrt/wavefront/`); RTG1 ch3 *Introduction to DirectX Raytracing*
(concepts); RTG2 ch14 (GPU reference path tracer);
[vk_mini_path_tracer](https://github.com/nvpro-samples/vk_mini_path_tracer),
[vk_raytracing_tutorial_KHR](https://github.com/nvpro-samples/vk_raytracing_tutorial_KHR).

---

## Not on the path (yet)

From `ROADMAP.md`, parked:
- ray marching / SDFs (Glint Stage 14 covers them)
- non-photorealistic rendering
- WASM / GitHub Pages
- scene JSON (replaced by the pbrt loader, 16)
- KD-trees and grids
- SIMD packets
- MLT (pbrt-v3 §16.4) is a candidate stage after 25.
