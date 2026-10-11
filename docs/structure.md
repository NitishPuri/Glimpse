# Structure review (Stage 0)

Glimpse was written by hand in Feb–Mar 2025, following the *Ray Tracing in One Weekend* books. It was not
written with today's goals in mind:
- a pbrt-v4 reading spine
- 29 techniques that each stay selectable
- CPU renders as references for a later GPU port
- code that reads like pbrt (literate: the comments carry the math and the *why*)

This note checks the current layout against those goals and proposes changes. Glint is the model for
repo-level organisation. pbrt-v4 is the model for the renderer's own files.

**Status: approved 2026-10-11 — all five recommendations accepted (section 5).** Approved changes are applied as
behaviour-preserving moves and renames, with the tests green after each step, before Stage 1 starts.

---

## 1. What is there today

```
src/core/            32 files, flat: math, camera, film, image, materials, textures, pdfs, scenes, renderer,
  hittables/         sphere, quad, moving_sphere, constant_medium, bvh_node, hittable_list, hittable(+instancing)
src/cli/main.cpp     Glimpse_cli
src/gui/             Glimpse_gui (GLFW + ImGui)
tests/unit_tests/    16 *_test.cpp, each a void foo_test() called by hand from tests/testing.cpp
tests/e2e/           image comparison suite (disabled), 848-line ASCII image printer
ext/                 boost.ut, stb, imgui+implot, glad (vendored); ext.7z + prebuilt WIN64 GLFW (legacy)
res/earthmap.jpg     the only asset
glimpse_results/     viewer "Save Image" output (gitignored)
results/             CLI output (gitignored)
test_output/         test scratch output in the repo root (gitignored)
log_cli.txt, log_gui.txt, imgui.ini   written into the repo root
CMakeSettings.json, .vscode/, .github/workflows/, *.bat   Windows/VS-era tooling
```

What works well and should stay:
- The **core library / CLI / viewer / tests split**: four targets, a clean dependency direction.
- **Small headers per concept** (`ray.h`, `interval.h`, `aabb.h`, `onb.h`, `pdf.h`), close to how pbrt
  splits things.
- **Own math types** (`vec3`, `interval`, `aabb`). pbrt writes its own too, and it keeps the math visible.
  (The 2025 roadmap ticked "use glm", but glm is only referenced in dead code. That is fine: no change.)
- A unit-test culture: 16 test files, about 1300 checks.

## 2. Findings

### 2.1 Layout doesn't map to the reading spine

Each technique's **Read** section points at a pbrt-v4 file (`samplers.h`, `bxdfs.h`, `lights.h`, `media.h`,
`cpu/integrators.cpp`, `util/sampling.h`, ...). Glimpse's flat `src/core/` has no matching places:
- The integrator (`ray_color`) lives in `render.cpp`, mixed with threading and the film.
- The pdfs and sampling warps live in `pdf.h` and `vec3.h` (`random_unit_vector`, `random_cosine_direction`).
- Materials and textures share no folder with what they replace later (BSDFs in technique 11).

As Part A lands (samplers, an iterative integrator, NEE/MIS, BSDFs), `src/core` would grow into a flat folder
of about 50 files with no landmarks.

### 2.2 Naming is RTIOW snake_case, while the reading spine and Glint are not

| | Types | Functions | Members | Files |
|---|---|---|---|---|
| Glimpse today | `hit_record`, `bvh_node`, `camera` (plus outliers `Scene`, `Random`, `Renderer`, `Film`, `Image`, `Logger`) | `ray_color`, `random_double` | `image_width`, `m_Width` (mixed) | snake_case |
| pbrt-v4 | `SurfaceInteraction`, `BVHAggregate` | `SampleLi`, `Sample_f` | `camelCase` | lowercase |
| Glint | `PascalCase` | `camelCase` | `m_camelCase` | snake_case |

Part A ports pbrt concepts one after another. Writing those in pbrt style next to the RTIOW-style code
would leave the codebase **mixed**, which is the worst option for reading.

### 2.3 Dead and parked code

| Item | State | Proposal |
|---|---|---|
| `src/core/ray_analysis.cpp`, `ray_visualizer.{h,cpp}`, `src/gui/ray_debug.h` | `#if 0`, references paths that never existed (`glimpse/debug/...`) | delete. Technique 10 (`path_inspector`) is written fresh; git history keeps them |
| `src/gui/plots.h` | included nowhere (pdf/π-estimate experiments) | delete, or move to `tools/` if worth keeping |
| `tests/e2e/print_ascii_image.h` (848 lines) | debugging aid for the disabled e2e suite | delete with the e2e rewrite in Stage 1 |
| `tests/e2e/` | disabled, flaky, references gitignored | replaced in Stage 1 (01–03) by float regression tests against `refs/` |
| about 80 lines of commented-out code (mostly `scenes.cpp`, `e2e_test.cpp`, `image.cpp`, `render.cpp`, `ui.cpp`) | experiments | delete; the git history keeps them |
| 24 TODOs | mixed | each one either goes into `docs/PLAN.md` (e.g. `film.h` variance → 05, texture filtering → later), gets fixed if trivial, or is deleted |
| `ext/ext.7z`, `ext/glfw-3.4.bin.WIN64` (untracked), `CMakeSettings.json`, `.github/workflows/*` (stale; CI skipped) | legacy | delete |
| `src/core/logger.h` | MSVC `#pragma warning`, outside the namespace | tidy when it moves (section 3) |

### 2.4 Code conventions

- `using namespace glimpse;` at global scope in 6 `.cpp` files, and **in a header** (`gui/raytracer.h`).
  Definitions should sit inside `namespace glimpse { ... }`.
- The GUI code has no namespace.
- One concept per file isn't consistent:
  - `hittable.h` also holds `translate` and `rotate_y`.
  - `common.h` mixes math constants, `Random` and the `random_*` helpers.
  - `camera.h` carries GUI manipulators (`fly`, `orbit`; its own TODO says to move them out).
- **The scene registry is duplicated:**
  - `Scene::SceneMap` (name → factory) and `Scene::SceneNames` (order) must be kept in sync by hand.
  - Two scenes have a "TODO: add light".
  - All 13 scenes live in one 517-line file.
- **Registering a test takes three places:** the CMake list, a forward declaration and a call in `testing.cpp`.
  boost.ut can self-register suites (`ut::suite<"vec3"> _ = [] {...};`), which would remove the manual list.
- **Runtime output lands in the repo root** (`log_*.txt`, `imgui.ini`, `test_output/`), plus two different
  results folders (`results/` for the CLI, `glimpse_results/` for the viewer).

### 2.5 Readability (the literate goal)

The comments are RTIOW-sparse. They say *what*, rarely the estimator, the pdf or the measure. Examples:
- `ray_color` doesn't say which integral it estimates, or why the mixture pdf is unbiased.
- `pdf.h` doesn't name the measure its pdfs are in (solid angle).
- `camera.h` has a TODO asking whether `vertical` should be inverted (it caused the upside-down bug fixed in
  Stage 0).

pbrt-v4 and Glint both open each file with what it is and where it comes from, and comment the reasoning
at each non-obvious step.

## 3. Proposed layout

Library code is split like pbrt-v4, so a technique's **Read** link and the Glimpse file have the same name. The
repo-level split follows Glint (`docs/`, `tools/`, `cmake/`). Folders are created only when something goes into
them: nothing is pre-created for Stage 5+.

```
src/glimpse/               the renderer library (include as "glimpse/<file>.h"), namespace glimpse
  util/                    vecmath.h (Vec3, ONB), interval.h, aabb.h, rng.h (Random), sampling.h (warps,
                           pdfs), image.{h,cpp}, log.h, cli_options.{h,cpp}
  camera.h                 camera model only (manipulators move to gui/)
  film.h
  shapes.{h,cpp}           sphere, moving_sphere, quad (+ box())           ← hittables/*
  aggregates.{h,cpp}       hittable_list, BVH                               ← hittables/
  instancing.h             translate, rotate_y                              ← hittable.h
  interaction.h            hit_record (pbrt: SurfaceInteraction), the hittable interface
  media.h                  constant_medium                                  ← hittables/
  materials.h              lambertian, metal, dielectric, diffuse_light, isotropic
  textures.{h,cpp}         + perlin.h
  integrators.{h,cpp}      ray_color → the integrator; Renderer keeps threading/film
  scenes/                  registry.{h,cpp} (one ordered table), plus one file per family:
                           rtiow.cpp (random, earth, perlin, ...), cornell.cpp, showcase.cpp
src/cli/main.cpp
src/gui/                   namespace glimpse::gui; camera manipulators move here
tests/                     test_<file>.cpp mirroring src/glimpse/, self-registering boost.ut suites
docs/                      PLAN.md, structure.md, techniques/NN_name.md
tools/                     scripts (compare/convergence plots arrive in Stage 1)
cmake/                     deps.cmake (FetchContent: GLFW today, later OIDN, ...)
ext/                       vendored single-file libraries (as in Glint's external/)
res/                       assets
refs/   results/   logs/   runtime output, all gitignored (logs/ replaces log_*.txt in the root;
                           the viewer saves into results/ like the CLI)
```

Smaller conventions that go with it:
- **Naming:** see decision D1.
- **File headers:** every file starts with a short comment block covering what it is, which pbrt-v4 section
  or RTIOW chapter it follows, and anything non-obvious about its conventions. These get written during the
  move: cheap, and a big win for reading.
- **No `using namespace` at global scope** anywhere in `src/`.
- **Scenes** are declared in one ordered table (`{name, factory}`), so the CLI index, the viewer combo and
  `--list-scenes` all read the same list.
- **Tests** run in `build/<cfg>/` scratch space, not the repo root.

## 4. Order of work (after approval)

Each step is one reviewable chunk, and the tests stay green after each:

1. **Delete dead and legacy files** (section 2.3): the `#if 0` files, `plots.h`, `ext.7z`, `CMakeSettings.json`,
   the workflows. No behaviour change. *(done 2026-10-11)*
2. **Runtime output:** `logs/`, `results/` for the viewer too, tests writing into the build tree, no
   `imgui.ini`. *(done 2026-10-11)*
3. **Moves and file splits** into the section 3 layout, with namespace hygiene (no renames inside files yet).
   Includes become `"glimpse/..."`.
   - 3a *(done 2026-10-11)*: `src/core` → `src/glimpse` (+ `util/`), every include a full `"glimpse/..."` path.
   - 3b-i *(done 2026-10-11)*: `hittables/` → `interaction.h`, `instancing.h`, `shapes`, `aggregates`, `media.h`;
     `materials.h`, `textures`; `ray_color` → `integrators.{h,cpp}`.
   - 3b-ii: util splits (`vecmath`, `sampling`, `rng`, `log`), `glimpse.cpp` statics to their types, namespace
     hygiene (no global `using namespace`, `glimpse::gui`), `ext/` as system includes, the pre-existing warnings.
4. **Scene registry** as a single ordered table; scene families split into files.
5. **Tests self-register** (boost.ut suites); the manual list in `testing.cpp` goes.
6. **Naming** per D1, as a mechanical rename. If D1 = keep, this step is dropped.
7. **File-header comments** across the tree, plus resolving the TODOs and the commented-out code. Each item
   is decided in context; some are alternative scene setups.

Behaviour changes found on the way (like the bugs in Stage 0) are listed separately, not mixed into the moves.

## 5. Decisions for the owner

- **D1 — naming convention.**
  - (a) Switch to pbrt/Glint style now: `PascalCase` types, `camelCase` functions, plain `camelCase` members as in
    pbrt (or `m_` as in Glint). One mechanical rename before Stage 1, so every pbrt port that follows matches.
  - (b) Keep RTIOW snake_case, and accept that pbrt ports get translated into it.
  - *Recommendation: (a), with pbrt-style plain members.* The reading spine is pbrt, and most core code gets
    rewritten technique by technique in Part A anyway. Renaming once now is cheaper than living with a mix.
- **D2 — layout.** The section 3 layout (pbrt-like file split), or keep `src/core` flat for now and split
  later. *Recommendation: section 3.*
- **D3 — dead code.** Delete the `#if 0` files, `plots.h` and the commented-out blocks (history keeps them),
  or move them to an `attic/` folder. *Recommendation: delete.*
- **D4 — tests.** Self-registering boost.ut suites, or keep boost.ut with the manual list, or switch to
  doctest like Glint. *Recommendation: self-registering boost.ut. It's already here, and it removes the
  three-place registration.*
- **D5 — Windows tooling.** Keep the `.bat` scripts and the `win` preset (untested), or drop Windows support
  until it's needed. *Recommendation: keep the `.bat` scripts, drop `CMakeSettings.json` and the stale
  workflows.*
