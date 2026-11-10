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
  `rayColor`, `Material::scatter`, `shared_ptr`, `double`) changes only at the technique that requires it, and that
  refactor is written up in the technique note.
- **Converge on pbrt-v4's class model** (owner's request, 2026-10-17): `Integrator` (07), `Light` / `LightSampler`
  (08), `Shape` / `Primitive` (14), `Sampler` (04), `Filter` (06), `BSDF` (11), spectral (19), media (20). The
  "pbrt alignment" table in `docs/architecture.md` tracks each gap. Every technique note has a "pbrt alignment"
  section. Don't build new features on the RTIOW shortcuts a planned refactor is about to replace.
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
  - pbrt-v4's `imgtool diff` can cross-check error metrics (02 found exact agreement). It is already built on
    Linux at `.../pbrt/pbrt-v4/build/linux/imgtool`; use it, don't rebuild elsewhere:
    `imgtool diff --metric MRSE --reference refs/<scene>.pfm x.pfm` (also `MSE`; it reads EXR too). `build/` is
    gitignored in pbrt. To rebuild (about 270 steps): `cmake -S pbrt-v4 -B pbrt-v4/build/linux -G Ninja
    -DCMAKE_BUILD_TYPE=Release -DGLFW_BUILD_WAYLAND=OFF && ninja -C pbrt-v4/build/linux imgtool` (Wayland is off
    because `libxkbcommon-dev` isn't installed). The `pbrt` renderer target lives in the same tree if a technique
    ever needs pbrt's own render as a reference.
- **Code is for reading.** The owner learns from it, so comment the *why* (the estimator, the pdf, the measure
  conversion), not the *what*.
- **Diagrams** (as in Glint), wherever a picture beats words: mermaid inline in the docs for flow and structure,
  and computed matplotlib figures from `tools/diagrams.py` (deterministic SVG in `docs/diagrams/`, using the same
  formulas as the code) for pdfs, warps, sample patterns, BSDF curves and convergence plots. See `docs/PLAN.md`,
  "Diagrams". `docs/architecture.md` has the module graph, the render flow and the `rayColor` bounce as mermaid,
  plus the first figures. Keep it current when a technique changes the picture.

Obsidian vault (`/mnt/d/ObsidianVault`, use the obsidian-kb skill):
- `1_TOPICS/Tech/Tech Projects/Glimpse/`: `Glimpse - Learning Path.md` and one `Glimpse - Stage N - <name>.md` per
  stage. When a technique lands, fill in that stage's Learnings section and tick its reading items.
- `1_TOPICS/Tech/Computer Graphics/Ray Tracing/`:
  - `Glimpse.md` is the project hub; it has a progress log.
  - `PBRT v4 - Reading Index.md` is the pbrt section checklist, tagged by technique number.
- `1_TOPICS/Tech/Tech Projects/Glint/` has the sibling project's notes.
- Older design chats live in `8_CHATS/chatgpt/` (BVH, concurrency, noise, roadmap review).

## Environment

- Now on **Pop!_OS 22.04** with cmake 3.22 and ninja. The system `c++` is GCC 11.4, which has almost no C++23
  library, so builds use **GCC 14** (`g++-14`, ubuntu-toolchain-r PPA) and **Clang 20** (apt.llvm.org), picked by
  `CMakePresets.json` (see [C++ standard](#c-standard) and [Tooling](#tooling)). The repo is on an NTFS mount, so
  expect slow builds and every file showing mode 755.
- Line endings are LF (`.gitattributes`; `.bat` files stay CRLF). `.git-blame-ignore-revs` lists the
  line-ending-only commits.
- Build temporaries live only in `build/<preset>/` (gitignored): `build/release`, `build/debug`,
  `build/clang-debug` (Linux presets, `build.sh`) and `build/windows` (preset `win`, the `.bat` scripts, untested
  since the move to Linux; leave them alone until a Windows build is tried again). Tests write to
  `build/<preset>/test_output`.
- `ext/`: boost.ut, stb, ImGui + ImPlot and glad are vendored. GLFW 3.4 is fetched by CMake (FetchContent,
  pinned like Glint). The old prebuilt Windows GLFW is no longer used.

## Build & run

Linux (same pattern as Glint):
```bash
./build.sh                            # preset `release` (GCC 14) into build/release; also: debug, clang-debug
./run.sh cli --scene cornell_box      # runs from the repo root; CFG=debug selects the debug build
./run.sh gui
./run.sh tests "camera*"              # argv[1] is a boost.ut name filter; or: ctest --test-dir build/release
```
Windows (old setup): `configure.bat` → `build.bat`; `cli.bat`, `gui.bat`, `test.bat <filter>`.

State (Stage 0 done; **Stage 1: 01 `float_output` and 02 `error_metrics` done**, next 03 `deterministic_rng`; technique notes in `docs/techniques/`): the library, CLI, viewer and tests build as C++23 with GCC 14 and Clang 20 with zero warnings (the viewer's own
sources included), clang-tidy is clean, and the tests pass under ctest.
- CLI flags: `--scene <name|index> --spp --width --depth --seed --out --list-scenes --help`.
  - `--spp` is rounded down to a square (the stratified grid is `sqrtSpp × sqrtSpp`).
  - `--seed` is reproducible on one machine, but not across thread counts (technique 03).
  - `--out` takes `.png/.jpg/.bmp/.tga` (sRGB, `--exposure`) or `.pfm` (linear radiance). `--reference` renders
    4096 spp into `refs/<scene>.pfm`.
  - `compare <test.pfm> <ref.pfm> [--error-image x.png]` prints MSE/RMSE/relMSE (pbrt's MRSE, `util/metrics`).
    `--convergence x.csv` renders 1..`--spp` (default 1024) spp against the reference (size taken from it) and
    writes `spp,seconds,mse,rmse,relmse`; `tools/diagrams.py` plots it from `docs/data/`.
  - `refs/cornell_box.pfm` is 300×300 (`--width 300`, seed 101); test renders need that width.
- `Renderer::renderScene` calls `cam.initialize()` itself, and the stop flag aborts capped renders too.
- `Image` rows are top-down, like image files: the renderer converts from camera `v` when writing, and the viewer
  doesn't flip.
- Scenes declare their own `scene.lights`.
- The viewer window opens at 80% of the screen. `src/gui/layout.h` pins a 420 px sidebar and an output panel
  every frame, and `imgui.ini` is not used.
- No CI: it was skipped on purpose, and `ctest` runs locally.

Style: `.clang-format` (Google base, 2-space indent, 120 columns). Everything is in `namespace glimpse`.

## C++ standard

The project builds as **C++23** (`CMAKE_CXX_STANDARD 23`, extensions off) and requires GCC ≥ 14, Clang ≥ 19 or
MSVC ≥ 19.37 (`CMakeLists.txt` enforces it). **New and touched code uses the C++23 (and C++20) idiom where one
exists**, as long as it reads at least as clearly as the old form (code is for reading). Don't sweep untouched
files just to modernise them; convert the code you are editing. `clang-tidy`'s `modernize-*` checks flag most of
the old spellings (see [Tooling](#tooling)).

| Use | Instead of | Where it already is |
|---|---|---|
| `std::format`, `std::print`/`println`; `log::info("{} spp", n)` (compile-time checked) | `iostream` chains, `ostringstream`, `to_string` | `log.h`, `cli/main.cpp`, `render.cpp`, `image.cpp` |
| chrono formatting of `localNow<Precision>()` (`"{:%Y%m%d_%H%M%S}"`) | `localtime` + `put_time` | `log.h`, the viewer's Save Image |
| `std::expected<T, std::string>` for failures that are a normal outcome | result struct with an `error` string; `optional` that loses the reason | `parseCommandLine`, `parseCompareCommand`, `readPFM` |
| exceptions for failures that end the run | `expected` that every caller just passes up | escape `main` on purpose (`NOLINT` there) |
| `std::views::zip(a, b)` | index loops over parallel arrays | `compareImages` (`metrics.cpp`) |
| `std::views::enumerate(v)` | `for (size_t i...)` when `i` is only a label/second index | `--list-scenes`, the viewer's scene combo |
| `std::ranges::contains`, `find` (with projections: `&SceneEntry::name`), `fill`, `transform` | `\|\|` chains, hand-written search loops, iterator pairs | `cli_options.cpp` (`kValueFlags`, `kWritable`), `findScene`, `Image::clear` |
| `str.contains`, `starts_with`/`ends_with` | `find(...) != npos`, `rfind(x, 0) == 0`, `substr(size() - n)` | `cli_options.cpp`, tests |
| monadic `optional` (`transform`, `and_then`, `value_or`) | `if (opt) return f(*opt); return nullopt;` | `resolveSceneName` |
| `std::numbers::pi`, `sqrt2`, ... | literal constants | `util/math.h`, `convergence.cpp` |
| `std::min`/`std::max` for clamps | `if (t0 > t.min) t.min = t0;` (same NaN behaviour) | the slab test (`aabb.cpp`), `ConstantMedium` |
| `std::cmp_less`, `cmp_greater_equal`, ... | `static_cast<int>(v.size())` to silence sign compares | `RayTracer::setupScene` |
| `shared_ptr` parameters: by value + `std::move` when stored, `const&` when only read | by-value copies (an atomic inc/dec each) | shapes, materials, textures, `HittableList`, BVH comparators |
| `0uz`, `3uz` | `size_t{0}`, widening after an `int` multiply | image tests |

Available and expected when the code calls for them (nothing uses them yet): `std::to_underlying(e)` instead of
`static_cast<int>(e)` (ImGui radio buttons), `std::unreachable()` after a `switch` that covers every enumerator
(keeps `-Wswitch` working), `range | std::ranges::to<std::vector>()` instead of `push_back` loops that only build a
vector, deducing `this` instead of const/non-const overload pairs.

Leave the explicit form where the index *is* the lesson: the row split across threads in `render.cpp`, the
stratified `sqrtSpp × sqrtSpp` grid, pixel loops, Perlin's permutation tables, BVH construction. Filtered
`push_back` loops (the convergence schedule, the viewer's plot series) also stay: as pipelines they read worse.
`Vec3`'s `operator[]` stays a plain const/non-const pair, and its `operator<<` stays because boost.ut prints values
with it.

Not available yet on this toolchain (libstdc++ 14): `std::mdspan`, `std::flat_map`, formatting of ranges
(`std::format("{}", vec)`; `image_test.cpp`'s `rgbText()` spells the channels out). When the compiler changes,
re-probe with `<version>` feature-test macros (`__cpp_lib_*`) and update this section; don't assume from release
notes.

## Tooling

- **Presets** (`CMakePresets.json`): `release`, `debug` (GCC 14) and `clang-debug` (Clang 20 + lld), each into
  `build/<preset>`; `win` (VS 2022, `build/windows`) only shows on Windows. `./build.sh <preset>` configures and
  builds, and first drops a `CMakeCache.txt` made by another compiler (an old GCC 11 tree fails the first
  reconfigure otherwise). Build `clang-debug` before committing larger changes: a second compiler finds different
  warnings (Clang found the Perlin tests that asserted nothing).
- **Warnings**: `-Wall -Wextra -pedantic` on the library, CLI, tests and the viewer's own sources (not vendored
  ImGui/ImPlot/glad). Keep all three presets at zero.
- **clangd** (`.clangd`): reads `build/debug/compile_commands.json` (so build `debug` once); runs cheap clang-tidy
  checks as you type and flags unused/missing includes. Needs clangd ≥ 19 (`clangd-20`): clangd 18 can't parse
  libstdc++ 14's `<expected>` and reports false errors.
- **clang-tidy** (`.clang-tidy`): `modernize-*`, `bugprone-*`, `performance-*` + a few readability checks; the file
  explains each disabled check. The tree is clean, so keep it clean: fix new findings, or `// NOLINT(check)` with
  the reason. Whole tree (about 3 minutes on the NTFS mount):
  `run-clang-tidy-20 -p build/debug -quiet '/(src/(glimpse|gui|cli)|tests/unit_tests)/'`; changed files:
  `clang-tidy-20 -p build/debug <files>`.
- **clang-format** only the lines you changed (files aren't all formatted, and whole-file formatting buries the
  diff): `git diff -U0 --no-color | clang-format-diff-20 -p1 -i -binary clang-format-20`.
- **Build-time profiling**: `cmake --preset clang-debug -DCMAKE_CXX_FLAGS=-ftime-trace`, then open the per-TU
  `.json` next to each `.o` in `chrome://tracing` / Perfetto (which headers make the NTFS build slow).

## Architecture

Targets: `Glimpse` (static library, a recursive glob over `src/glimpse`), `Glimpse_cli` (`src/cli/main.cpp`),
`Glimpse_gui` (`src/gui`) and `Glimpse_tests`.

**Rendering pipeline** (`src/glimpse/render.cpp`, integrator in `integrators.cpp`):
- `Renderer::renderScene(Scene, Image&, progress*)` builds a `BVHNode` from `scene.world` on every render.
  It then splits the image **by rows across `hardware_concurrency()` threads** using `std::async`.
- Each thread loops over stratified sub-pixel cells (`cam.sqrtSpp`²), then over its rows and pixels. It calls
  `rayColor(...)` and adds the sample to the shared `Film`. Then it writes the running mean, sRGB-encoded
  (`util/color.h`), into `Image` via `setFloat`. The GUI uploads that same buffer while rendering is still going.
- Output comes from the linear film: `Film::linearRGB()` gives a `FloatImage`. `--out x.pfm` writes it as-is;
  8-bit formats get `encodeSRGB(linear, --exposure)`.
- `cam.uncappedSpp` switches to `renderSectionUncap`, which repeats passes until the static
  `Renderer::stopRequested` flag is set (the GUI's stop button).
- `rayColor` is recursive. Emission plus scatter: specular materials use `srec.skipPdf`. Otherwise it
  samples a `MixturePDF` (light `HittablePDF` + material pdf) **only if `scene.lights` is non-empty**.
  Lights are a separate `HittableList` that must duplicate emitter geometry from `world`.
- The viewer's `ErrorTracker` (`src/gui/error_tracker.h`) plots live relMSE against `refs/<scene>.pfm` (ImPlot),
  but only while the render matches the reference (size, camera, depth, background). Progress is `atomic<int64_t>`.
- `Film` keeps per-pixel sample counts and sums. The Welford mean/variance code exists but is commented
  out, so variance always reads zero. Adaptive sampling depends on it.
- `Random` (`util/rng.h`) uses a thread-local `mt19937` with a global seed (`Random::setSeed`, 0 = random).
  Determinism across thread counts is not guaranteed.

**Scenes** (`src/glimpse/scenes/`): a `Scene` (`scene.h`) is world + lights + background + `Camera`. Factories live in
family files (`rtiow.cpp`, `cornell.cpp`, `showcase.cpp`), are declared in `factories.h`, and are listed once in
`registry.cpp`. `sceneRegistry()` is an ordered table that the CLI index, `--list-scenes` and the viewer combo all
read, so new scenes go at the end. `findScene(name)` looks one up by name.

**Utilities** (`src/glimpse/util/`): `math.h` (constants, `degreesToRadians`), `rng.h` (`Random`, `randomDouble`),
`vecmath.h` (`Vec3`/`Point3`/`Color`, `ONB`), `sampling.h` (warps: `randomUnitVector`, `randomCosineDirection`,
...), `interval.h`, `AABB`, `image`, `log.h` (thread-safe `log::debug/info/warn/error` free functions like
Glint's; `log::setFile(logFileFor(app))`; debug lines only in debug builds), `cli_options`. The viewer code is in
`namespace glimpse::gui`. Nothing in `src/` has a global `using namespace`.

**Geometry** (pbrt-like split): `interaction.h` (`HitRecord` + the `Hittable` interface), `shapes.h`
(`Sphere`, whose moving-center constructor gives motion blur; `Quad` + `box()`), `aggregates.{h,cpp}` (`HittableList`, `BVHNode`), `instancing.h`
(`Translate`, `RotateY`), `media.h` (`ConstantMedium`). Materials (`materials.h`) return a `ScatterRecord` holding
either a pdf or a `skipPdfRay`; textures are in `textures.{h,cpp}`.

**GUI** (`src/gui`): `AppWindow` owns `GLResources` (GLFW window, GL texture that shows the `Image`), `UI`
(ImGui/ImPlot panels: scene choice, camera, SPP, depth, progress, plots, saving to `glimpse_results/`) and
`RayTracer` (runs `renderScene` on a `std::async` future; the main thread polls `status`/`progress`).

**Structure:** `docs/structure.md` is the approved structure review (2026-10-11). It moves the library to
`src/glimpse/` with a pbrt-like file split and pbrt/Glint naming (`PascalCase` types, `camelCase` functions and
members), self-registering tests, and runtime output in `logs/` and `results/`. Follow it for new code, even
while the move is in progress.

## Tests

- boost.ut (`ext/boost/ut.hpp`, header only, `BOOST_UT_DISABLE_MODULE`). Each file in `tests/unit_tests/` registers
  its own suites: `static ut::suite<"name"> name_suite = [] { using namespace boost::ut; ... };`. CMake globs the test
  files (re-run configure after adding one), and `tests/testing.cpp` only sets the filter and calls `run()`.
- Tests that write files use `testOutputPath("x.png")` (`build/<cfg>/test_output`).
- `tests/e2e/` (compiled, but **not registered**, so it doesn't run) renders small scenes (`test_scenes.h`) with a fixed seed and compares them against
  `tests/e2e/reference_images/` (gitignored, so they must be generated locally) with a per-channel tolerance.
  The ROADMAP notes these comparisons are flaky because of 8-bit encoding loss and threading. The planned
  fix is float output for comparisons.
