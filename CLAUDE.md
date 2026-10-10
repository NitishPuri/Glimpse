# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**Glimpse** is a CPU path tracer in C++ and the owner's **ray tracing learning project**. It grew out of
`ray-tracer-in-a-weekend` and covers all three *Ray Tracing in One Weekend* books (BVH, textures, Perlin
noise, quads, volumes, motion blur, DOF, importance sampling + MIS toward lights). On top of that it adds a
GLFW/ImGui viewer with progressive ("uncapped SPP") rendering and a boost.ut test suite. It was last
developed in Feb–Mar 2025 on Windows/MSVC.

It is the ray tracing companion to **Glint** (`/mnt/e/tree/graphics/glint`): an OpenGL 4.6 + Vulkan 1.3
side-by-side renderer that works toward GPU Gems chapters. Glint's Stage 15 does ray-traced shadows and AO
inside a rasterizer, while Glimpse's Part C does the full path tracer on the GPU. Read Glint's `CLAUDE.md` and
`docs/PLAN.md` before planning GPU work, so the two projects don't duplicate each other.

Since 2026-10-10 the laptop runs in `nvidia` graphics mode, so GL and VK run on the **RTX 3060**, which has
hardware ray tracing. The AMD iGPU can still be selected for Vulkan with `VK_LOADER_DRIVERS_SELECT='*radeon*'`.

## The plan: read `docs/PLAN.md` first

`docs/PLAN.md` is the learning path, agreed 2026-10-10. It has three parts:
- **A:** measure, then reduce noise
- **B:** content and advanced light transport
- **C:** GPU port, with the CPU renders as references

It is split into stages and numbered techniques. Work in its order, and tick its checkboxes in the same commit
as the work. `ROADMAP.md` is only the 2025 history.

Rules that come with it:
- **One new idea per technique; refactors only when a technique needs them.** The RTIOW-shaped core (recursive
  `ray_color`, `material::scatter`, `shared_ptr`, `double`) changes only at the technique that requires it, and that
  refactor is written up in the technique note.
- **Old methods stay selectable** behind flags (`--integrator`, `--sampler`, ...) so every technique can be
  compared at equal time against what it replaces.
- **Definition of done**, as listed in `PLAN.md`:
  - a float (PFM) render of the technique's scene
  - RMSE/relMSE against a high-SPP reference in `refs/`
  - an equal-time comparison
  - `docs/techniques/NN_name.md`, using the template in `PLAN.md`
  - tests pass
- **Reading:** pbrt-v4 (<https://pbr-book.org/4ed/>, local source `/mnt/e/tree/graphics/ray_tracing/pbrt/pbrt-v4`)
  plus Ray Tracing Gems I/II articles. For BDPT and SPPM, read pbrt-v3 ch16 and the local `pbrt-v3` source. Each
  technique note has a **Read** section with links.
  - Local pbrt-v4 scenes for testing are in `.../pbrt/pbrt_scenes/pbrt-v4-scenes`.
  - pbrt-v4's `imgtool diff` can cross-check error metrics.
- **Code is for reading.** The owner learns from it, so comment the *why* (the estimator, the pdf, the measure
  conversion), not the *what*.

Obsidian vault (`/mnt/d/ObsidianVault`, use the obsidian-kb skill):
- `1_TOPICS/Tech/Tech Projects/Glimpse/`: `Glimpse - Learning Path.md` and one `Glimpse - Stage N - <name>.md` per
  stage. When a technique lands, fill in that stage's Learnings section and tick its reading items.
- `1_TOPICS/Tech/Computer Graphics/Ray Tracing/`:
  - `Glimpse.md` is the project hub; it has a progress log.
  - `PBRT v4 - Reading Index.md` is the pbrt section checklist, tagged by technique number.
- `1_TOPICS/Tech/Tech Projects/Glint/` has the sibling project's notes.
- Older design chats live in `8_CHATS/chatgpt/` (BVH, concurrency, noise, roadmap review).

## Environment

- Now on **Pop!_OS 22.04, GCC 11.4** (so no `std::format`), with cmake and ninja available. The repo is on an
  NTFS mount, so expect slow builds and every file showing mode 755.
- The Windows helper scripts (`*.bat`, `CMakePresets.json` preset `win`) and the committed `build/` and
  `buildLinux/` trees are from the old Windows setup. All `build*` dirs are gitignored.
- `ext/`: only `boost/ut.hpp` and `stb/` are tracked. `glad/`, `imgui/` (with ImPlot) and the prebuilt
  `glfw-3.4.bin.WIN64` are gitignored but archived in `ext/ext.7z`.

## Build & run

Linux:
```bash
cmake -S . -B buildLinux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build buildLinux
./buildLinux/Glimpse_cli --scene 6        # index into Scene::SceneNames
./buildLinux/Glimpse_tests                # all suites
./buildLinux/Glimpse_tests "camera*"      # argv[1] is a boost.ut name filter
```
Windows (old setup): `configure.bat` → `build.bat`; `cli.bat`, `gui.bat`, `test.bat <filter>`.

Known state on Linux (checked 2026-10-03):
- The **GUI is not built on Linux**. CMake skips `Glimpse_gui` when `LINUX` and hardwires the WIN64 GLFW
  libs plus `opengl32.lib`. Glint already did this port (system GLFW + `OpenGL::GL`) and can be copied.
- **`Glimpse_tests` fails to link under GCC**: there are multiple definitions of `ut::cfg<ut::override>`. It is
  explicitly specialized in `tests/testing.cpp`, but other TUs have already used the primary template.
  MSVC tolerated this. The library and CLI build fine.
- The CLI only honours `--scene`. `--width/--height/--samples` are parsed but ignored, because resolution
  and SPP come from the scene's camera. `main.cpp` always adds the Cornell-box ceiling light quad to
  `scene.lights` and writes to `./results/final21.jpg`.
- The GitHub workflows are disabled or stale: `cpp-tests.yml` targets branch `main` and runs `test.bat`
  on Ubuntu.

Style: `.clang-format` (Google base, 2-space indent, 120 columns). Everything is in `namespace glimpse`.

## Architecture

Targets: `Glimpse` (static library, a recursive glob over `src/core`), `Glimpse_cli` (`src/cli/main.cpp`),
`Glimpse_gui` (`src/gui`, Windows only) and `Glimpse_tests`.

**Rendering pipeline** (`src/core/render.cpp`):
- `Renderer::render_scene(Scene, Image&, progress*)` builds a `bvh_node` from `scene.world` on every render.
  It then splits the image **by rows across `hardware_concurrency()` threads** using `std::async`.
- Each thread loops over stratified sub-pixel cells (`cam.sqrt_spp`²), then over its rows and pixels. It calls
  `ray_color(...)` and adds the sample to the shared `Film`. Then it writes the running mean, gamma-corrected
  with `sqrt`, into `Image` via `set_float`. The GUI uploads that same buffer while rendering is still going.
- `cam.uncapped_spp` switches to `render_section_uncap`, which repeats passes until the static
  `Renderer::stop_rendering` flag is set (the GUI's stop button).
- `ray_color` is recursive. Emission plus scatter: specular materials use `srec.skip_pdf`. Otherwise it
  samples a `mixture_pdf` (light `hittable_pdf` + material pdf) **only if `scene.lights` is non-empty**.
  Lights are a separate `hittable_list` that must duplicate emitter geometry from `world`.
- `Film` keeps per-pixel sample counts and sums. The Welford mean/variance code exists but is commented
  out, so variance always reads zero. Adaptive sampling depends on it.
- `Random` (`common.h`) uses a thread-local `mt19937` with a global seed (`Random::set_seed`, 0 = random).
  Determinism across thread counts is not guaranteed.

**Scenes** (`scenes.cpp`): each scene is a free function that returns a `Scene` (world, lights,
background, `camera`). It is registered in `Scene::SceneMap` (name → factory) **and** in `Scene::SceneNames`. The
order of that vector is the CLI `--scene` index and the GUI's combo order. A new scene needs both entries.

**Hittables** (`core/hittables/`): `hittable` interface (+ `translate`/`rotate_y` instancing wrappers),
`sphere`, `moving_sphere`, `quad` (+ `box()` helper), `constant_medium`, `hittable_list`, `bvh_node`.
Materials (`material.h`) return a `scatter_record` holding either a pdf or a `skip_pdf_ray`.

**GUI** (`src/gui`): `AppWindow` owns `GLResources` (GLFW window, GL texture that shows the `Image`), `UI`
(ImGui/ImPlot panels: scene choice, camera, SPP, depth, progress, plots, saving to `glimpse_results/`) and
`RayTracer` (runs `render_scene` on a `std::async` future; the main thread polls `status`/`progress`).

**Dormant code**: `ray_analysis.cpp`, `ray_visualizer.*` and `gui/ray_debug.h` are wrapped in `#if 0`.
They are the start of the roadmap's per-ray path visualisation and instrumentation, and do not compile.

## Tests

- boost.ut (`ext/boost/ut.hpp`, header only, `BOOST_UT_DISABLE_MODULE`). Each file in `tests/unit_tests/`
  defines a `void <name>_test()` function. `tests/testing.cpp` forward-declares and calls every one of them.
- Test sources are **listed explicitly** in CMakeLists (no glob). A new test file needs a CMake entry, a
  forward declaration and a call in `testing.cpp`.
- `tests/e2e/` renders small scenes (`test_scenes.h`) with a fixed seed and compares them against
  `tests/e2e/reference_images/` (gitignored, so they must be generated locally) with a per-channel tolerance.
  The ROADMAP notes these comparisons are flaky because of 8-bit encoding loss and threading. The planned
  fix is float output for comparisons.
