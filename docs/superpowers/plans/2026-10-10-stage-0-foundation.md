# Stage 0 — Foundation on Linux: Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** On Linux, a clean clone of Glimpse builds with one command, its tests pass under `ctest`, the
viewer runs, and `Glimpse_cli --scene cornell_box --spp 64 --out x.png` writes a real image.

**Architecture:**
- **No new rendering.** This stage repairs what broke when the 2025 Windows/MSVC project moved to GCC 11.
- **Bugs fixed:** the test link error, a CLI that renders a black image (the camera is never initialised), a
  stray hardcoded light, every written image being upside down, and Windows-only GUI linking.
- **CLI parsing moves into the core library** (`src/core/cli_options.{h,cpp}`) so the unit tests can cover it.
- **Dependencies:** GLFW comes from FetchContent (pinned exactly as in Glint). ImGui/ImPlot and glad, which
  sit untracked in `ext/`, become vendored, so a clean clone needs nothing extra.

**Tech Stack:** C++20 (GCC 11.4, so no `std::format`), CMake 3.22 + Ninja, boost.ut 2.3.0 (`ext/boost/ut.hpp`),
GLFW 3.4, Dear ImGui 1.91.9 + ImPlot 0.17, glad (GL loader), stb.

**Spec:** `docs/PLAN.md`, section "Stage 0 — Foundation on Linux". Background on the repo: `CLAUDE.md`.

## Global Constraints

- Line endings are **LF** (decided 2026-10-10), enforced by `.gitattributes`. `.bat` files stay CRLF.
- Compiler is GCC 11.4: do not use `std::format`, `<print>`, `std::expected` or ranges `to`.
- Style: `.clang-format` (Google, 2-space indent, 120 columns). Everything lives in `namespace glimpse`.
- Tests use boost.ut. Test sources are **listed explicitly** in `CMakeLists.txt`, and each suite function is
  forward-declared and called in `tests/testing.cpp`.
- `src/core` is compiled with `file(GLOB_RECURSE ...)`, so **re-run the configure step** after adding a `.cpp`
  there (`cmake --preset linux`, or `cmake -S . -B buildLinux` before Task 7 adds the preset).
- Run every binary **from the repo root**: scenes load `./res/earthmap.jpg`, and logs go to `./log_*.txt`.
- No new rendering features. Anything belonging to techniques 01+ (PFM output, sRGB, deterministic seeding)
  stays out.
- One commit per task, with messages ending in the `Co-Authored-By` line from the session's attribution rules.

## Review Focus

1. **Non-square `--spp`.** Stratified sampling renders `floor(sqrt(spp))²` samples. `--spp 10` renders 9, and
   the CLI must report the real count, not the requested one. Test: Task 3, `apply_spp_rounds_down`.
2. **`--width` changes the height.** The output `Image` must match the camera's recomputed
   `image_width × image_height`, or the render writes out of bounds. Test: Task 3, `apply_width_recomputes_height`.
3. **An output path in a missing directory** (`--out renders/a/b.png`) should create the directories. An
   extension `Image::write` cannot produce (`.pfm`, `.exr`, none) must be rejected up front, not silently
   written as PNG. Tests: Task 3, `out_extension_validation`; Task 5, manual step 4.
4. **Scene names vs indices.** `--scene 9` means `Scene::SceneNames[9]`. `-1`, `99` and unknown names are
   errors that point to `--list-scenes`; they must not crash or index out of range. Test: Task 3, `resolve_scene`.
5. **`ctest` must actually fail when a test fails.** The old pass/fail regexes never matched this reporter.
   Test: Task 2, step 5 (a planted failure must turn ctest red).

---

## File map

| File | Change | Responsibility |
|---|---|---|
| `.gitattributes`, `.git-blame-ignore-revs` | create | LF policy; hide the two line-ending-only commits from blame |
| `tests/test_cfg.h` | modify | the one shared definition of the boost.ut runner |
| `tests/testing.cpp` | modify | test entry point (remove the demo tests that fail on purpose) |
| `src/core/render.cpp` | modify | `render_scene` initialises the camera itself |
| `tests/unit_tests/render_test.cpp` | modify | regression tests: black CLI image, upside-down images |
| `src/gui/ui.cpp` | modify | drop the display-time flip once `Image` rows are top-down |
| `src/core/cli_options.h`, `src/core/cli_options.cpp` | create | parse and validate options, resolve scenes, apply overrides |
| `src/core/cli.h` | delete (Task 5) | old header-only parser, replaced by `cli_options` |
| `tests/unit_tests/cli_test.cpp` | create | unit tests for `cli_options.h` |
| `src/cli/main.cpp` | rewrite | thin CLI front end over `cli_options.h` |
| `CMakeLists.txt` | modify | test target cleanup, GUI on every platform, compile commands |
| `.gitignore`, `ext/imgui/**`, `ext/glad/**` | modify / vendor | clean clone needs no extracted archive |
| `CMakePresets.json` | modify | `linux` preset (Ninja, `buildLinux`) |
| `.github/workflows/*` | optional | Linux build+test job, manual trigger |
| `CLAUDE.md`, `docs/PLAN.md` | modify | build commands and known state; tick Stage 0 |

---

### Task 1: LF line endings

**Files:**
- Create: `.gitattributes`, `.git-blame-ignore-revs`

**Interfaces:** none (repository hygiene). It goes first so every later diff is free of line-ending noise.

- [ ] **Step 1: Check the tree is clean**

Run: `git status --short`
Expected: no output. If anything is listed, stop and ask the owner. The renormalize step rewrites the
working tree.

- [ ] **Step 2: Write `.gitattributes`**

```gitattributes
# LF everywhere (decided 2026-10-10, matches Glint). Windows batch files need CRLF to run in cmd.
* text=auto eol=lf
*.bat text eol=crlf

*.7z binary
*.png binary
*.jpg binary
*.jpeg binary
*.bmp binary
*.tga binary
*.hdr binary
*.exr binary
*.pfm binary
```

- [ ] **Step 3: Renormalize and commit**

```bash
git add .gitattributes
git add --renormalize .
git status --short | wc -l      # expect roughly 85 files (the ones the 2026-10-02 "crlf" commit touched)
git commit -m "Normalize line endings to LF via .gitattributes"
```

- [ ] **Step 4: Rewrite the working tree with LF**

```bash
git checkout-index --force --all
git status --short              # expect: no output
file CMakeLists.txt README.md ROADMAP.md build.bat
```
Expected: `CMakeLists.txt`, `README.md` and `ROADMAP.md` have no "CRLF" in their description. `build.bat` says
"with CRLF line terminators".

- [ ] **Step 5: Hide both line-ending commits from blame, and commit**

```bash
{ echo "# Line-ending-only commits (git config blame.ignoreRevsFile .git-blame-ignore-revs)";
  git rev-parse 163a5cc;  # "crlf", 2026-10-02
  git rev-parse HEAD; } > .git-blame-ignore-revs
git config blame.ignoreRevsFile .git-blame-ignore-revs
git add .git-blame-ignore-revs
git commit -m "Ignore line-ending-only commits in git blame"
```
Then check: `git blame -s CMakeLists.txt | head -3`. The commit hashes shown must not be either of the two
ignored ones.

---

### Task 2: Test suite links, passes, and fails properly under ctest

GCC fails with `multiple definition of boost::ext::ut::v2_3_0::cfg<override>`. `tests/testing.cpp` explicitly
specialises the variable template `ut::cfg<ut::override>`, but every other test file has already used the
primary template, which is an ODR violation that MSVC tolerated. The fix: define the specialisation once,
`inline`, in `tests/test_cfg.h`. Every test file includes that header right after `ut.hpp`, so all files see
one runner. This was verified on a scratch copy on 2026-10-10: 35 tests, 1286 asserts, all pass.

**Files:**
- Modify: `tests/test_cfg.h` (after the `namespace ut = boost::ut;` line)
- Modify: `tests/testing.cpp` (lines 1–62: specialisation, `sum`, `example_tests`; plus the call in `main`)
- Modify: `CMakeLists.txt` (the `# Tests` block)

**Interfaces:**
- Produces: `ut::cfg<ut::override>` (a `runner<reporter<printer>>`), defined only in `tests/test_cfg.h`.
  Task 3 adds a suite under the same rules.

- [ ] **Step 1: Confirm the failure**

```bash
cmake -S . -B buildLinux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build buildLinux --target Glimpse_tests 2>&1 | grep -m1 "multiple definition"
```
Expected: one line containing `multiple definition of` ... `cfg<boost::ext::ut::v2_3_0::override>`.

- [ ] **Step 2: Move the runner into `tests/test_cfg.h`**

Directly after the line `namespace ut = boost::ut;`, insert:

```cpp

// The one runner every test file reports to. It must be `inline` and visible before any test file uses
// ut::cfg, otherwise each file instantiates its own copy (ODR violation; GCC refuses to link, MSVC didn't).
template <>
inline auto ut::cfg<ut::override> = ut::runner<ut::reporter<ut::printer>>{};
```

- [ ] **Step 3: Strip the specialisation and the demo tests from `tests/testing.cpp`**

`example_tests()` is boost.ut's tutorial code. It fails on purpose (`fatal(v[4] = 4)`) and divides by zero,
which raises SIGFPE on Linux. Replace everything from the top of the file down to and including the closing
`}` of `example_tests()` with:

```cpp
#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

void vec3_test();
void ray_test();
void film_test();
void aabb_test();
void texture_test();
void image_test();
void test_image_roundtrip();
void interval_test();
void onb_test();
void pdf_test();
void perlin_test();
void camera_test();
void render_test();
void sphere_test();
void bvh_test();
void random_test();

// End-to-end tests
void e2e_test();
```

In `main`, delete the line `  example_tests();` and the blank line after it. Leave the rest of `main`
unchanged; `e2e_test()` stays commented out until technique 03.

- [ ] **Step 4: Clean up the CMake test block**

In `CMakeLists.txt`, replace the whole block from `# Tests` through its `endif()` with the block below. It
drops the duplicate `texture_test.cpp`, the stale "Does not work!!" comment and the regex properties that
never matched:

```cmake
# Tests
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    message(STATUS "Building tests")

    enable_testing()

    add_library(boost_ut INTERFACE)
    target_include_directories(boost_ut INTERFACE ${PROJECT_SOURCE_DIR}/ext)

    add_executable(Glimpse_tests
        tests/test_cfg.h
        tests/testing.cpp

        tests/unit_tests/random_test.cpp
        tests/unit_tests/vec3_test.cpp
        tests/unit_tests/ray_test.cpp
        tests/unit_tests/aabb_test.cpp
        tests/unit_tests/texture_test.cpp
        tests/unit_tests/image_test.cpp
        tests/unit_tests/film_test.cpp
        tests/unit_tests/interval_test.cpp
        tests/unit_tests/onb_test.cpp
        tests/unit_tests/pdf_test.cpp
        tests/unit_tests/perlin_test.cpp
        tests/unit_tests/camera_test.cpp
        tests/unit_tests/render_test.cpp
        tests/unit_tests/sphere_test.cpp
        tests/unit_tests/bvh_node_test.cpp

        tests/e2e/e2e_test.cpp
        tests/e2e/test_scenes.h
    )
    target_link_libraries(Glimpse_tests
        PRIVATE
        ${NAME}
        boost_ut
    )
    target_include_directories(Glimpse_tests
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext
    )

    # boost.ut's runner calls std::exit(-1) when any assertion failed, so ctest can rely on the exit code.
    # Run from the repo root: tests read ./res and write ./test_output.
    add_test(NAME GlimpseTests COMMAND Glimpse_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
```

- [ ] **Step 5: Build, run, and prove ctest turns red on a failure**

```bash
cmake --build buildLinux
./buildLinux/Glimpse_tests | tail -2
ctest --test-dir buildLinux --output-on-failure | tail -3
```
Expected: `All tests passed (1286 asserts in 35 tests)`, `4 tests skipped`, and ctest
`100% tests passed, 0 tests failed out of 1`. The assert count may differ slightly; zero failures is what matters.

Now plant a failure and check ctest catches it. Add `expect(false) << "planted";` as the first line inside
the outer `"vec3"_test = [] {` lambda in `tests/unit_tests/vec3_test.cpp`, then:

```bash
cmake --build buildLinux && ctest --test-dir buildLinux | tail -2
git checkout -- tests/unit_tests/vec3_test.cpp
```
Expected: ctest reports `0% tests passed, 1 tests failed`. After the `git checkout`, rebuild, and ctest is
green again.

Also check the name filter: `./buildLinux/Glimpse_tests "camera*" | tail -1` should print
`All tests passed (18 asserts in 1 tests)`.

- [ ] **Step 6: Commit**

```bash
git add tests/test_cfg.h tests/testing.cpp CMakeLists.txt
git commit -m "Fix Glimpse_tests link error on GCC; ctest uses the exit code"
```

---

### Task 3: `render_scene` initialises the camera; CLI options library

Bug: `Glimpse_cli` never calls `scene.cam.initialize()`, so `cam.image_height` stays 0. The render covers
rows [0, 0] in under a millisecond and writes a black image (reproduced 2026-10-10). The GUI calls
`initialize()` itself. The robust fix is for `Renderer::render_scene` to initialise its by-value copy of
the camera, so no caller can forget. `initialize()` only recomputes derived fields, so calling it twice is
harmless.

In the same task, CLI parsing gets a tested library unit, `src/core/cli_options.{h,cpp}`. The old header-only
`src/core/cli.h` (a non-inline function defined in a header) stays untouched until Task 5 replaces its only
user, `main.cpp`. That way every commit builds.

**Files:**
- Modify: `src/core/render.cpp`, in `Renderer::render_scene` (first line of the function body)
- Modify: `tests/unit_tests/render_test.cpp` (new case inside `"renderer"_test`)
- Create: `src/core/cli_options.h`
- Create: `src/core/cli_options.cpp`
- Create: `tests/unit_tests/cli_test.cpp`
- Modify: `tests/testing.cpp` (declare and call `cli_test`)
- Modify: `CMakeLists.txt` (add `tests/unit_tests/cli_test.cpp` to `Glimpse_tests`)

**Interfaces:**
- Consumes: `Scene::SceneMap`, `Scene::SceneNames` (`src/core/scenes.h`); `camera::initialize()`.
- Produces (used by Task 5), all in `namespace glimpse`, header `core/cli_options.h`:
  - `struct CmdOptions { std::string scene = "cornell_box"; std::optional<int> spp, width, depth; std::optional<uint32_t> seed; std::string out; bool list_scenes = false; bool help = false; };`
  - `struct ParseResult { CmdOptions options; std::string error; bool ok() const; };`
  - `ParseResult parse_command_line(int argc, const char* const argv[]);`
  - `std::string usage();`
  - `std::optional<std::string> resolve_scene_name(const std::string& name_or_index);`
  - `void apply_options(const CmdOptions& options, Scene& scene);` (applies overrides, then `cam.initialize()`)
  - `std::string default_output_path(const std::string& scene_name);` (returns `"results/<scene>.png"`)

- [ ] **Step 1: Write the failing render regression test**

In `tests/unit_tests/render_test.cpp`, inside the `"renderer"_test = [] { ... }` lambda, add this case
**before** the existing `skip / "render_scene"_test`:

```cpp
    // Regression (2026-10-10): Glimpse_cli never called cam.initialize(), so image_height stayed 0 and
    // render_scene produced an all-black image. render_scene must initialise the camera itself.
    "render_scene_initializes_camera"_test = [] {
      Scene scene;
      scene.cam.aspect_ratio = 2.0;
      scene.cam.image_width = 8;  // image_height deliberately left at 0
      scene.cam.samples_per_pixel = 1;
      scene.cam.max_depth = 1;
      scene.cam.defocus_angle = 0.0f;
      scene.background = color(1, 1, 1);  // every camera ray misses, so every pixel is white
      // One object behind the camera, because the BVH needs at least one primitive.
      scene.world.add(make_shared<sphere>(point3(0, 0, 100), 0.1, make_shared<lambertian>(color(0.5, 0.5, 0.5))));

      Image image(8, 4);
      Renderer renderer;
      renderer.render_scene(scene, image);

      int lit = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i)
          if (image.get(i, j).rgb[0] > 0) ++lit;
      expect(lit == 32_i) << "every pixel should see the white background";
    };
```

- [ ] **Step 2: Run it and watch it fail**

```bash
cmake --build buildLinux && ./buildLinux/Glimpse_tests "renderer*" | tail -4
```
Expected: FAIL at `render_scene_initializes_camera` with `0 == 32`.

- [ ] **Step 3: Fix `render_scene`**

In `src/core/render.cpp`, make this the first statement of `void Renderer::render_scene(Scene scene, Image &image, std::atomic<int> *progress) {`:

```cpp
  // Derived camera fields (image_height, sqrt_spp, viewport) must be current. Callers set public fields and
  // may forget initialize(); it only recomputes, so calling it again is harmless.
  scene.cam.initialize();
```

- [ ] **Step 4: Run it and watch it pass**

```bash
cmake --build buildLinux && ./buildLinux/Glimpse_tests "renderer*" | tail -2
```
Expected: `All tests passed`. The old `render_scene` case is still listed as skipped.

- [ ] **Step 5: Write the failing CLI tests**

Create `tests/unit_tests/cli_test.cpp`:

```cpp
#include "core/cli_options.h"

#include <initializer_list>
#include <vector>

#include "core/scenes.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
ParseResult parse(std::initializer_list<const char*> args) {
  std::vector<const char*> argv{"Glimpse_cli"};
  argv.insert(argv.end(), args.begin(), args.end());
  return parse_command_line(static_cast<int>(argv.size()), argv.data());
}
}  // namespace

void cli_test() {
  using namespace boost::ut;

  "cli"_test = [] {
    "defaults"_test = [] {
      auto r = parse({});
      expect(r.ok()) << r.error;
      expect(r.options.scene == "cornell_box");
      expect(!r.options.spp.has_value() && !r.options.width.has_value() && !r.options.depth.has_value());
      expect(!r.options.seed.has_value());
      expect(r.options.out.empty());
      expect(!r.options.help && !r.options.list_scenes);
    };

    "all_flags"_test = [] {
      auto r = parse({"--scene", "earth", "--spp", "16", "--width", "320", "--depth", "8", "--seed", "42", "--out",
                      "renders/e.png"});
      expect(r.ok()) << r.error;
      expect(r.options.scene == "earth");
      expect(r.options.spp.value_or(-1) == 16_i);
      expect(r.options.width.value_or(-1) == 320_i);
      expect(r.options.depth.value_or(-1) == 8_i);
      expect(r.options.seed.value_or(0u) == 42_u);
      expect(r.options.out == "renders/e.png");
    };

    "help_and_list"_test = [] {
      expect(parse({"--help"}).options.help);
      expect(parse({"-h"}).options.help);
      expect(parse({"--list-scenes"}).options.list_scenes);
      expect(usage().find("--spp") != std::string::npos);
    };

    "rejects_bad_input"_test = [] {
      expect(!parse({"--spp"}).ok()) << "missing value";
      expect(!parse({"--spp", "abc"}).ok()) << "not a number";
      expect(!parse({"--spp", "12abc"}).ok()) << "trailing garbage";
      expect(!parse({"--spp", "0"}).ok()) << "zero samples";
      expect(!parse({"--width", "-5"}).ok()) << "negative width";
      expect(!parse({"--width", "100000"}).ok()) << "absurd width";
      expect(!parse({"--seed", "0"}).ok()) << "0 means 'random' in Random::set_seed";
      expect(!parse({"--bogus"}).ok()) << "unknown flag";
      expect(parse({"--bogus"}).error.find("--bogus") != std::string::npos) << "error names the flag";
    };

    "out_extension_validation"_test = [] {
      expect(parse({"--out", "a.png"}).ok());
      expect(parse({"--out", "a.JPG"}).ok());
      expect(parse({"--out", "a.jpeg"}).ok());
      expect(parse({"--out", "dir/a.bmp"}).ok());
      expect(parse({"--out", "a.tga"}).ok());
      expect(!parse({"--out", "a.pfm"}).ok()) << "PFM arrives in technique 01";
      expect(!parse({"--out", "a.exr"}).ok());
      expect(!parse({"--out", "noext"}).ok());
    };

    "resolve_scene"_test = [] {
      expect(resolve_scene_name("cornell_box").value_or("") == "cornell_box");
      expect(resolve_scene_name("9").value_or("") == Scene::SceneNames[9]);
      expect(resolve_scene_name("0").value_or("") == Scene::SceneNames[0]);
      expect(!resolve_scene_name("-1").has_value());
      expect(!resolve_scene_name(std::to_string(Scene::SceneNames.size())).has_value());
      expect(!resolve_scene_name("no_such_scene").has_value());
      expect(!resolve_scene_name("").has_value());
    };

    "apply_keeps_scene_values"_test = [] {
      Scene s = Scene::SceneMap["cornell_box"]();  // 600 px, 100 spp, aspect 1.0
      apply_options(CmdOptions{}, s);
      expect(s.cam.image_width == 600_i);
      expect(s.cam.image_height == 600_i) << "initialize() ran";
      expect(s.cam.samples_per_pixel == 100_i);
      expect(s.cam.sqrt_spp == 10_i);
    };

    "apply_spp_rounds_down"_test = [] {
      Scene s = Scene::SceneMap["cornell_box"]();
      CmdOptions o;
      o.spp = 10;
      apply_options(o, s);
      expect(s.cam.samples_per_pixel == 10_i);
      expect(s.cam.sqrt_spp == 3_i) << "a 3x3 stratified grid: 9 samples actually rendered";
    };

    "apply_width_recomputes_height"_test = [] {
      Scene s = Scene::SceneMap["earth"]();
      CmdOptions o;
      o.width = 101;
      o.depth = 7;
      apply_options(o, s);
      expect(s.cam.image_width == 101_i);
      expect(s.cam.image_height == static_cast<int>(101 / s.cam.aspect_ratio));
      expect(s.cam.max_depth == 7_i);
    };

    "default_output_path"_test = [] { expect(default_output_path("earth") == "results/earth.png"); };
  };
}
```

Register it in `tests/testing.cpp`: add `void cli_test();` after `void random_test();`, and call
`  cli_test();` after `  random_test();` in `main`. In `CMakeLists.txt`, add
`        tests/unit_tests/cli_test.cpp` after `tests/unit_tests/bvh_node_test.cpp`.

- [ ] **Step 6: Watch it fail to compile**

```bash
cmake --build buildLinux --target Glimpse_tests 2>&1 | grep -m3 "error"
```
Expected: errors such as `'ParseResult' does not name a type` / `'resolve_scene_name' was not declared`.

- [ ] **Step 7: Write `src/core/cli_options.h`**

```cpp
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace glimpse {

struct Scene;

// Command-line options. Unset optionals mean "keep the scene's own value".
struct CmdOptions {
  std::string scene = "cornell_box";  // a name from Scene::SceneMap, or an index into Scene::SceneNames
  std::optional<int> spp;
  std::optional<int> width;  // height follows from the scene camera's aspect ratio
  std::optional<int> depth;
  std::optional<uint32_t> seed;  // >= 1; 0 is reserved by Random::set_seed for "random"
  std::string out;               // empty: default_output_path(scene)
  bool list_scenes = false;
  bool help = false;
};

struct ParseResult {
  CmdOptions options;
  std::string error;  // empty on success
  bool ok() const { return error.empty(); }
};

ParseResult parse_command_line(int argc, const char* const argv[]);
std::string usage();

// Accepts a scene name or a decimal index into Scene::SceneNames.
std::optional<std::string> resolve_scene_name(const std::string& name_or_index);

// Applies the overrides to the scene's camera, then recomputes its derived fields (image_height, sqrt_spp, ...).
void apply_options(const CmdOptions& options, Scene& scene);

std::string default_output_path(const std::string& scene_name);

}  // namespace glimpse
```

- [ ] **Step 8: Write `src/core/cli_options.cpp`**

```cpp
#include "cli_options.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <sstream>
#include <string_view>

#include "scenes.h"

namespace glimpse {

namespace {

// The whole string must be a base-10 integer in [min, max]: rejects "", "12abc", "-5" for unsigned ranges, overflow.
template <class T>
std::optional<T> parse_int(std::string_view text, long long min, long long max) {
  long long value = 0;
  const char* end = text.data() + text.size();
  auto [ptr, ec] = std::from_chars(text.data(), end, value);
  if (ec != std::errc{} || ptr != end || value < min || value > max) return std::nullopt;
  return static_cast<T>(value);
}

// Formats Image::write can produce today. PFM arrives with technique 01 (float_output).
bool is_writable_extension(std::string_view path) {
  auto dot = path.find_last_of('.');
  auto slash = path.find_last_of('/');
  if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash)) return false;
  std::string ext(path.substr(dot + 1));
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga";
}

}  // namespace

ParseResult parse_command_line(int argc, const char* const argv[]) {
  ParseResult result;
  CmdOptions& o = result.options;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      o.help = true;
      continue;
    }
    if (arg == "--list-scenes") {
      o.list_scenes = true;
      continue;
    }
    if (arg != "--scene" && arg != "--spp" && arg != "--width" && arg != "--depth" && arg != "--seed" &&
        arg != "--out") {
      result.error = "unknown argument: " + arg;
      return result;
    }
    if (i + 1 >= argc) {
      result.error = "missing value for " + arg;
      return result;
    }
    const std::string value = argv[++i];
    const std::string invalid = "invalid value for " + arg + ": '" + value + "'";

    if (arg == "--scene") {
      o.scene = value;
    } else if (arg == "--out") {
      if (!is_writable_extension(value)) {
        result.error = invalid + " (supported: .png .jpg .jpeg .bmp .tga)";
        return result;
      }
      o.out = value;
    } else if (arg == "--seed") {
      o.seed = parse_int<uint32_t>(value, 1, std::numeric_limits<uint32_t>::max());
      if (!o.seed) {
        result.error = invalid + " (1 .. 4294967295)";
        return result;
      }
    } else {
      // Generous upper bounds: they only catch typos, not taste.
      const long long max = arg == "--width" ? 16384 : arg == "--spp" ? 1'000'000 : 10'000;
      auto v = parse_int<int>(value, 1, max);
      if (!v) {
        result.error = invalid + " (1 .. " + std::to_string(max) + ")";
        return result;
      }
      (arg == "--spp" ? o.spp : arg == "--width" ? o.width : o.depth) = v;
    }
  }
  return result;
}

std::string usage() {
  std::ostringstream ss;
  ss << "usage: Glimpse_cli [--scene <name|index>] [--spp N] [--width N] [--depth N] [--seed N] [--out file]\n"
        "                   [--list-scenes] [--help]\n"
        "  --scene   scene name or index (default cornell_box; see --list-scenes)\n"
        "  --spp     samples per pixel; rounded down to a square (10 -> 9) for stratified sampling\n"
        "  --width   image width; height follows the scene's aspect ratio\n"
        "  --depth   maximum bounces\n"
        "  --seed    fixed RNG seed (>= 1); reproducible on the same machine and thread count\n"
        "  --out     output image: .png .jpg .bmp .tga (default results/<scene>.png)\n";
  return ss.str();
}

std::optional<std::string> resolve_scene_name(const std::string& name_or_index) {
  if (Scene::SceneMap.count(name_or_index)) return name_or_index;
  auto index = parse_int<size_t>(name_or_index, 0, static_cast<long long>(Scene::SceneNames.size()) - 1);
  if (index) return Scene::SceneNames[*index];
  return std::nullopt;
}

void apply_options(const CmdOptions& options, Scene& scene) {
  if (options.spp) scene.cam.samples_per_pixel = *options.spp;
  if (options.width) scene.cam.image_width = *options.width;
  if (options.depth) scene.cam.max_depth = *options.depth;
  scene.cam.initialize();
}

std::string default_output_path(const std::string& scene_name) { return "results/" + scene_name + ".png"; }

}  // namespace glimpse
```

- [ ] **Step 9: Re-configure (new file under the glob), build, run**

```bash
cmake -S . -B buildLinux
cmake --build buildLinux && ./buildLinux/Glimpse_tests "cli*" | tail -2
```
Expected: `All tests passed` for `cli`, and `Glimpse_cli` still builds (it still uses the old `cli.h`).

- [ ] **Step 10: Full suite**

```bash
cmake --build buildLinux && ctest --test-dir buildLinux --output-on-failure | tail -2
```
Expected: `100% tests passed`.

- [ ] **Step 11: Commit**

```bash
git add src/core/render.cpp src/core/cli_options.h src/core/cli_options.cpp tests/unit_tests/render_test.cpp \
        tests/unit_tests/cli_test.cpp tests/testing.cpp CMakeLists.txt
git commit -m "render_scene initialises the camera; tested CLI option parsing in core"
```

---

### Task 4: Images are written right side up

Found while verifying this plan (2026-10-10): every image `Image::write` produces comes out upside down. The
light is on the floor and the boxes hang from the ceiling. The renderer stores row `j` = camera `v` (counting
up from the bottom, as in RTIOW), but image files store the top row first. The viewer hid this by flipping the
uvs at display time (`ImGui::Image(..., ImVec2(0, 1), ImVec2(1, 0))`). The owner's Feb 2025 saves in
`glimpse_results/` are the right way up, so this broke during the 2025 refactors.

The fix: `Image` rows are top-down, like files. The renderer converts once when writing, and the viewer stops
flipping. This was verified on a scratch copy: the CLI's Cornell box and the viewer's Earth are both the right
way up.

**Files:**
- Modify: `tests/unit_tests/render_test.cpp` (new case inside `"renderer"_test`)
- Modify: `src/core/render.cpp` (the `image.set_float` call in `render_section` and in `render_section_uncap`)
- Modify: `src/gui/ui.cpp`, in `UIRenderer::renderOutput` (the `ImGui::Image` call)

**Interfaces:**
- Produces: the convention **`Image` row 0 = top of the picture**. Technique 01 (`float_output`) and every
  later image consumer rely on it.

- [ ] **Step 1: Write the failing test**

In `tests/unit_tests/render_test.cpp`, inside `"renderer"_test`, add before `skip / "render_scene"_test`:

```cpp
    // Image rows are top-down like image files (row 0 = top of the picture); the camera's v runs bottom-up.
    // Regression (2026-10-10): the CLI wrote every image upside down.
    "image_rows_are_top_down"_test = [] {
      Scene scene;
      scene.cam.aspect_ratio = 1.0;
      scene.cam.image_width = 16;
      scene.cam.samples_per_pixel = 1;
      scene.cam.max_depth = 1;
      scene.cam.vfov = 90.0f;
      scene.cam.defocus_angle = 0.0f;
      scene.background = color(0, 0, 0);
      // A light entirely in the upper half of the view.
      scene.world.add(make_shared<sphere>(point3(0, 0.5, -1), 0.3, make_shared<diffuse_light>(color(1, 1, 1))));

      Image image(16, 16);
      Renderer renderer;
      renderer.render_scene(scene, image);

      long top = 0, bottom = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i) (j < image.height / 2 ? top : bottom) += image.get(i, j).rgb[0];
      expect(top > 0 and bottom == 0) << "light must land in the top rows: top=" << top << " bottom=" << bottom;
    };
```

- [ ] **Step 2: Run it and watch it fail**

```bash
cmake --build buildLinux --target Glimpse_tests && ./buildLinux/Glimpse_tests "renderer*" | grep -E "FAILED|top="
```
Expected: FAIL, `top= 0  bottom= 5100` (the light lands in the bottom rows).

- [ ] **Step 3: Fix the renderer**

In `src/core/render.cpp`, `render_section`, replace

```cpp
          image.set_float(i, j, static_cast<float>(pixel_color.x()), static_cast<float>(pixel_color.y()),
                          static_cast<float>(pixel_color.z()));
```
with
```cpp
          // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
          image.set_float(i, cam.image_height - 1 - j, static_cast<float>(pixel_color.x()),
                          static_cast<float>(pixel_color.y()), static_cast<float>(pixel_color.z()));
```
Make the same change in `render_section_uncap`. Its copy is indented two more spaces:
```cpp
            // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
            image.set_float(i, cam.image_height - 1 - j, static_cast<float>(pixel_color.x()),
                            static_cast<float>(pixel_color.y()), static_cast<float>(pixel_color.z()));
```
`Film` keeps its own (i, j) indexing; only the `Image` write changes.

- [ ] **Step 4: Stop flipping in the viewer**

In `src/gui/ui.cpp`, `UIRenderer::renderOutput`, replace

```cpp
  // flip vertically
  ImGui::Image(ImTextureID(gl_res.framebufferTexture), calculatePanelSize(gl_res), ImVec2(0, 1), ImVec2(1, 0));
```
with
```cpp
  // Image rows are top-down and so is ImGui's default uv (0,0) -> (1,1): no flip needed.
  ImGui::Image(ImTextureID(gl_res.framebufferTexture), calculatePanelSize(gl_res));
```

- [ ] **Step 5: Run the tests and look at a render**

```bash
cmake --build buildLinux && ctest --test-dir buildLinux | tail -1
./buildLinux/Glimpse_cli --scene cornell_box --spp 16 --width 200 --out renders/up.png
```
Expected: `100% tests passed`. Open `renders/up.png` with the Read tool. The ceiling light must be at the
**top**, the green wall on the left and the red wall on the right, matching
`glimpse_results/20250226103652_cornell_box.jpg`. Then `rm -rf renders`. (The viewer gets its visual check in
Task 6 step 4: the Earth must have the north pole at the top.)

- [ ] **Step 6: Commit**

```bash
git add tests/unit_tests/render_test.cpp src/core/render.cpp src/gui/ui.cpp
git commit -m "Image rows are top-down: written images were upside down; viewer no longer flips"
```

---

### Task 5: `Glimpse_cli` front end

**Files:**
- Rewrite: `src/cli/main.cpp`
- Delete: `src/core/cli.h` (its only user was `main.cpp`)

**Interfaces:**
- Consumes: everything listed under Task 3 "Produces"; `Random::set_seed(uint32_t)` (`core/common.h`);
  `Image(int, int)` and `bool Image::write(const std::string&)` (`core/image.h`); `Logger` (`core/logger.h`);
  `Renderer::render_scene` (`core/render.h`).
- Produces: the user-facing CLI. Exit code 0 on success, 1 if writing the image fails, 2 on bad arguments or
  an unknown scene.

- [ ] **Step 1: Rewrite `src/cli/main.cpp`**

```cpp
#include <chrono>
#include <filesystem>
#include <iostream>

#include "core/cli_options.h"
#include "core/glimpse.h"
#include "core/logger.h"
#include "core/render.h"

using namespace glimpse;

const std::string log_file_path = "./log_cli.txt";

int main(int argc, char **argv) {
  const ParseResult parsed = parse_command_line(argc, argv);
  if (!parsed.ok()) {
    std::cerr << "error: " << parsed.error << "\n\n" << usage();
    return 2;
  }
  const CmdOptions &options = parsed.options;

  if (options.help) {
    std::cout << usage();
    return 0;
  }
  if (options.list_scenes) {
    for (size_t i = 0; i < Scene::SceneNames.size(); ++i) std::cout << i << "  " << Scene::SceneNames[i] << "\n";
    return 0;
  }

  const auto scene_name = resolve_scene_name(options.scene);
  if (!scene_name) {
    std::cerr << "error: unknown scene '" << options.scene << "' (see --list-scenes)\n";
    return 2;
  }

  Logger logger(log_file_path);
  // Seeds every worker thread's generator. Rows are split per thread, so this is reproducible on one machine;
  // seeding that is independent of the thread count is technique 03.
  if (options.seed) Random::set_seed(*options.seed);

  // Scenes declare their own lights (scene.lights); nothing is added here.
  Scene scene = Scene::SceneMap[*scene_name]();
  apply_options(options, scene);
  const camera &cam = scene.cam;
  Image image(cam.image_width, cam.image_height);

  // Stratified sampling uses a sqrt_spp x sqrt_spp grid: the real sample count is sqrt_spp^2.
  logger.log("Rendering ", *scene_name, " at ", cam.image_width, "x", cam.image_height, ", ", cam.sqrt_spp * cam.sqrt_spp,
             " spp, depth ", cam.max_depth);
  const auto start = std::chrono::high_resolution_clock::now();

  Renderer renderer;
  renderer.render_scene(scene, image, nullptr);

  const auto seconds =
      std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
  logger.log("Rendered in ", seconds, " s");

  const std::string out = options.out.empty() ? default_output_path(*scene_name) : options.out;
  const auto parent = std::filesystem::path(out).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  if (!image.write(out)) {
    logger.log("Failed to write ", out);
    return 1;
  }
  logger.log("Wrote ", out);
  return 0;
}
```

- [ ] **Step 2: Delete the old header, then build everything and rerun the tests**

```bash
git rm -q src/core/cli.h
grep -rn '"core/cli.h"\|ParseCommandLine' src tests   # expect: no output
cmake -S . -B buildLinux > /dev/null                   # a file left the src/core glob
cmake --build buildLinux 2>&1 | grep -E "error|warning: " ; ctest --test-dir buildLinux | tail -1
```
Expected: no errors. `100% tests passed`.

- [ ] **Step 3: Check the argument handling**

```bash
./buildLinux/Glimpse_cli --list-scenes | head -3          # 0  directions_test / 1  two_diffuse_spheres / ...
./buildLinux/Glimpse_cli --help | head -1                 # usage: Glimpse_cli ...
./buildLinux/Glimpse_cli --spp nope; echo "exit=$?"        # error: invalid value for --spp: 'nope' ... exit=2
./buildLinux/Glimpse_cli --scene 99; echo "exit=$?"        # error: unknown scene '99' (see --list-scenes) exit=2
./buildLinux/Glimpse_cli --out a.pfm; echo "exit=$?"       # error: invalid value for --out ... exit=2
```

- [ ] **Step 4: Check the renders (Stage 0 "done when")**

```bash
rm -rf renders results/cornell_box.png
./buildLinux/Glimpse_cli --scene cornell_box --spp 64 --width 300 --out renders/a/cornell.png; echo "exit=$?"
./buildLinux/Glimpse_cli --scene 5 --spp 10 --width 200; echo "exit=$?"
ls -la renders/a/cornell.png results/earth.png
```
Expected:
- Both exit with 0.
- The second log says `Rendering earth at 200x112, 9 spp`: the real count, not 10.
- `renders/a/` was created.
- The render takes noticeably longer than 1 ms.

Open `renders/a/cornell.png` with the Read tool. It must show the Cornell box (red left wall, green right
wall, two boxes, ceiling light), not a black image. Then:

```bash
./buildLinux/Glimpse_cli --scene cornell_box --spp 4 --width 64 --seed 7 --out renders/s1.png
./buildLinux/Glimpse_cli --scene cornell_box --spp 4 --width 64 --seed 7 --out renders/s2.png
cmp renders/s1.png renders/s2.png && echo identical
rm -rf renders
```
Expected: `identical`. A fixed seed is reproducible on the same machine.

- [ ] **Step 5: Commit**

```bash
git add src/cli/main.cpp   # the deletion of src/core/cli.h is already staged by git rm
git commit -m "Glimpse_cli: real flags, scene lights from the scene, output dirs, exit codes"
```

---

### Task 6: Viewer on Linux, with a responsive window

`src/gui` is already portable. A scratch build on 2026-10-10 compiled with zero source changes and rendered
`earth` in 3.3 s on the RTX 3060. Only CMake is Windows-only: it links the prebuilt `glfw-3.4.bin.WIN64` and
`opengl32.lib`, and it gets ImGui/glad from directories that are gitignored and only exist after
extracting `ext/ext.7z`.

**Files:**
- Modify: `CMakeLists.txt` (the `# gui` block, from `# Platform detection` through its `endif()`)
- Modify: `.gitignore`
- Vendor: `ext/imgui/**` (ImGui 1.91.9 WIP + ImPlot 0.17 + GLFW/OpenGL3 backends, about 4.2 MB), `ext/glad/**` (about 0.4 MB)
- Create: `src/gui/layout.h` (pure sizing math, no ImGui/GL), `tests/unit_tests/gui_layout_test.cpp`
- Modify: `src/gui/gl_res.cpp` (`initGL`), `src/gui/app.cpp` (`initApp`), `src/gui/ui.cpp` (`renderUI`, `renderOutput`)
- Modify: `tests/testing.cpp`, `CMakeLists.txt` (register `gui_layout_test`)

Responsive window, added at the owner's request (2026-10-10). The window was a fixed 1800×1600, taller than
the 1440 px screen, and the panel layout came from a stale Windows `imgui.ini`.
- The window opens at 80% of the monitor work area, centered.
- Each frame, the Control Panel is a left sidebar (420 px, at most a third of the width) and Render Output fills
  the rest. Both follow window resizes.
- `imgui.ini` is no longer used.

**Interfaces:**
- Consumes: the `Glimpse` library target; GLFW 3.4 (FetchContent, pinned like Glint's `cmake/deps.cmake`).
- Produces: the `Glimpse_gui` target on every platform. `BUILD_GUI=OFF` still skips it.
- Produces: `src/gui/layout.h`, namespace `glimpse::gui`:
  - `struct Rect { float x, y, w, h; };`
  - `struct Layout { Rect controls; Rect output; };`
  - `Layout compute_layout(float x, float y, float w, float h, float sidebar_w = 420.0f);`
  - `struct WindowSize { int w, h; };`
  - `WindowSize initial_window_size(int work_w, int work_h, int fallback_w, int fallback_h);`

- [ ] **Step 1: Replace the GUI block in `CMakeLists.txt`**

Delete everything from `# Platform detection` through the `endif()` that closes `if(BUILD_GUI AND NOT LINUX)`,
and put this in its place:

```cmake
# gui
option(BUILD_GUI "Build the GUI Viewer" ON)
if(BUILD_GUI)
    message(STATUS "Building GUI")

    # GLFW 3.4 from source, pinned by hash (same as Glint). X11 only: Wayland needs extra dev packages.
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
      URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
      URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01)
    FetchContent_MakeAvailable(glfw)
    set(OpenGL_GL_PREFERENCE GLVND)  # silences CMP0072; GLVND is the modern libOpenGL/libGLX split
    find_package(OpenGL REQUIRED)

    set(GUI_NAME ${NAME}_gui)
    set(GUI_PATH ${PROJECT_SOURCE_DIR}/src/gui)
    set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/ext/imgui)  # vendored: ImGui + ImPlot + glfw/opengl3 backends
    set(GLAD_PATH ${PROJECT_SOURCE_DIR}/ext/glad)    # vendored GL loader

    file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
    file(GLOB_RECURSE APP_HEADERS ${GUI_PATH}/*.h)
    file(GLOB_RECURSE APP_SOURCES ${GUI_PATH}/*.cpp)

    add_executable(${GUI_NAME} ${APP_SOURCES} ${APP_HEADERS} ${IMGUI_SOURCES} ${GLAD_PATH}/src/glad.c)
    target_compile_features(${GUI_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${GUI_NAME} PRIVATE ${NAME} glfw OpenGL::GL)
    target_include_directories(${GUI_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext
        ${GLAD_PATH}/include
    )
endif()
```

- [ ] **Step 2: Vendor ImGui and glad**

In `.gitignore`, delete the two lines `ext/glad/` and `ext/imgui/`. Keep `ext/glfw*`: the prebuilt Windows
GLFW is no longer used. Then:

```bash
git status --short ext/ | head      # ext/glad/ and ext/imgui/ now show as untracked
ls ext/imgui/implot.cpp ext/imgui/imgui_impl_glfw.cpp ext/imgui/imgui_impl_opengl3.cpp ext/glad/src/glad.c
```
Expected: all four files exist. If they don't (an older clone), extract `ext/ext.7z` into `ext/` first.

- [ ] **Step 3: Configure and build**

```bash
cmake -S . -B buildLinux -G Ninja -DCMAKE_BUILD_TYPE=Release 2>&1 | grep -E "Building GUI|Error"
cmake --build buildLinux --target Glimpse_gui 2>&1 | grep -E "error|FAILED"; ls -la buildLinux/Glimpse_gui
```
Expected: `-- Building GUI`, no errors, and the binary exists. The first configure downloads GLFW.

- [ ] **Step 4: Run it and look at it**

Screenshot **only the Glimpse window** (`import -window Glimpse`), never the root window: the owner's other
screens are private.

```bash
rm -f log_gui.txt
(timeout 25 ./buildLinux/Glimpse_gui > /dev/null 2>&1 &)
sleep 15
import -window Glimpse "$SCRATCHPAD/glimpse_gui.png"   # $SCRATCHPAD = the session scratchpad directory
sleep 12
grep -E "Rendering|generated" log_gui.txt; grep -ciE "fail|error" log_gui.txt
```
Expected: `Setting up scene ... earth`, `Image generated in ~3 s`, and an error count of `0`. Open the
screenshot with the Read tool: it must show the ImGui control panel and the rendered Earth **with the north
pole at the top** (Africa and Europe the right way up). Delete the screenshot afterwards. The window
(1800×1600) is taller than the 1440 px screen, so its bottom edge is cut off. That is cosmetic and out of
scope.

> Commit grouping note: do steps 1–4 and make the first two commits of step 11 right after step 4, before
> starting step 5. That way the "Build Glimpse_gui" commit contains only the GUI block.

- [ ] **Step 5: Write the failing layout tests**

Create `tests/unit_tests/gui_layout_test.cpp`:

```cpp
#include "gui/layout.h"

//
#include "../test_cfg.h"

using namespace glimpse::gui;

void gui_layout_test() {
  using namespace boost::ut;

  "gui_layout"_test = [] {
    "sidebar_and_output_fill_the_window"_test = [] {
      auto l = compute_layout(0, 20, 1600, 900);  // y = 20: below a menu bar / work-area offset
      expect(l.controls.x == 0.0_f && l.controls.y == 20.0_f);
      expect(l.controls.w == 420.0_f && l.controls.h == 900.0_f);
      expect(l.output.x == 420.0_f && l.output.y == 20.0_f);
      expect(l.output.w == 1180.0_f && l.output.h == 900.0_f);
    };

    "narrow_window_caps_sidebar_at_a_third"_test = [] {
      auto l = compute_layout(0, 0, 900, 600);
      expect(l.controls.w == 300.0_f);
      expect(l.output.x == 300.0_f && l.output.w == 600.0_f);
    };

    "initial_size_is_80_percent_of_work_area"_test = [] {
      auto s = initial_window_size(5120, 1410, 1800, 1600);
      expect(s.w == 4096_i && s.h == 1128_i);
    };

    "initial_size_never_exceeds_small_screens"_test = [] {
      auto s = initial_window_size(700, 500, 1800, 1600);
      expect(s.w <= 700_i && s.h <= 500_i);
      expect(s.w == 700_i && s.h == 500_i) << "minimum 640x480 is clamped to the work area";
    };

    "no_monitor_info_uses_fallback"_test = [] {
      auto s = initial_window_size(0, 0, 1800, 1600);
      expect(s.w == 1800_i && s.h == 1600_i);
    };
  };
}
```

Register it: `void gui_layout_test();` and a call `  gui_layout_test();` in `tests/testing.cpp` (after
`cli_test`), and `        tests/unit_tests/gui_layout_test.cpp` in the `Glimpse_tests` list in
`CMakeLists.txt`.

- [ ] **Step 6: Watch it fail**

```bash
cmake --build buildLinux --target Glimpse_tests 2>&1 | grep -m1 "error"
```
Expected: `fatal error: gui/layout.h: No such file or directory`.

- [ ] **Step 7: Write `src/gui/layout.h`**

```cpp
#pragma once

#include <algorithm>

// Viewer layout math, free of ImGui/GL so the unit tests can cover it.
namespace glimpse::gui {

struct Rect {
  float x, y, w, h;
};

struct Layout {
  Rect controls;  // left sidebar
  Rect output;    // render view, fills the rest
};

// (x, y, w, h) is the usable area (ImGui main viewport work area). The sidebar keeps a fixed width but never
// takes more than a third of a narrow window.
inline Layout compute_layout(float x, float y, float w, float h, float sidebar_w = 420.0f) {
  const float side = std::min(sidebar_w, w / 3.0f);
  return {{x, y, side, h}, {x + side, y, w - side, h}};
}

struct WindowSize {
  int w, h;
};

// Opening size: 80% of the monitor work area, at least 640x480 but never larger than the work area itself.
// Without monitor information (work area <= 0) fall back to the configured size.
inline WindowSize initial_window_size(int work_w, int work_h, int fallback_w, int fallback_h) {
  if (work_w <= 0 || work_h <= 0) return {fallback_w, fallback_h};
  const int w = std::min(work_w, std::max(640, static_cast<int>(work_w * 0.8f)));
  const int h = std::min(work_h, std::max(480, static_cast<int>(work_h * 0.8f)));
  return {w, h};
}

}  // namespace glimpse::gui
```

- [ ] **Step 8: Watch the tests pass**

```bash
cmake --build buildLinux --target Glimpse_tests && ./buildLinux/Glimpse_tests "gui_layout*" | tail -1
```
Expected: `All tests passed`.

- [ ] **Step 9: Use it in the viewer**

`src/gui/gl_res.cpp`, in `initGL`: add `#include "layout.h"` at the top. Replace the `glfwCreateWindow(...)`
line with:

```cpp
  // Open at 80% of the monitor's work area, centered; the window stays resizable and the layout follows it.
  int work_x = 0, work_y = 0, work_w = 0, work_h = 0;
  if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) glfwGetMonitorWorkarea(monitor, &work_x, &work_y, &work_w, &work_h);
  const auto size = glimpse::gui::initial_window_size(work_w, work_h, WINDOW_WIDTH, WINDOW_HEIGHT);

  window = glfwCreateWindow(size.w, size.h, "Glimpse", nullptr, nullptr);
```
After the `if (!window) { ... }` block, add:

```cpp
  if (work_w > 0) glfwSetWindowPos(window, work_x + (work_w - size.w) / 2, work_y + (work_h - size.h) / 2);
```

`src/gui/app.cpp`, in `initApp`: directly after `ImGui::CreateContext();`, add:

```cpp
  // The layout is computed every frame (layout.h); a saved imgui.ini would only fight it.
  ImGui::GetIO().IniFilename = nullptr;
```

`src/gui/ui.cpp`: add `#include "layout.h"` after `#include "app.h"`, plus this helper above
`UIRenderer::cameraUI`:

```cpp
namespace {
// Pins the next ImGui window to a rect of the responsive layout (re-applied every frame, so it follows resizes).
void place_next_window(const glimpse::gui::Rect& r) {
  ImGui::SetNextWindowPos(ImVec2(r.x, r.y), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(r.w, r.h), ImGuiCond_Always);
}

glimpse::gui::Layout current_layout() {
  const ImGuiViewport* vp = ImGui::GetMainViewport();
  return glimpse::gui::compute_layout(vp->WorkPos.x, vp->WorkPos.y, vp->WorkSize.x, vp->WorkSize.y);
}

constexpr ImGuiWindowFlags kPinned = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
}  // namespace
```

In `renderUI`, replace `  ImGui::Begin("Control Panel");` with:

```cpp
  place_next_window(current_layout().controls);
  ImGui::Begin("Control Panel", nullptr, kPinned);
```

In `renderOutput`, replace `  ImGui::Begin("Render Output");` with:

```cpp
  place_next_window(current_layout().output);
  ImGui::Begin("Render Output", nullptr, kPinned);
```

`calculatePanelSize` already fits the image to the panel at the render's aspect ratio, so nothing changes
there.

- [ ] **Step 10: Build, run, look, resize**

```bash
cmake --build buildLinux 2>&1 | grep -E "error|FAILED"; ctest --test-dir buildLinux | tail -1
rm -f log_gui.txt
(timeout 30 ./buildLinux/Glimpse_gui > /dev/null 2>&1 &)
sleep 12
import -window Glimpse "$SCRATCHPAD/gui_a.png"
xdotool search --name '^Glimpse$' windowsize 1200 800 2>/dev/null || wmctrl -r Glimpse -e 0,-1,-1,1200,800
sleep 3
import -window Glimpse "$SCRATCHPAD/gui_b.png"
sleep 16
grep -ciE "fail|error" log_gui.txt
```
Expected: `100% tests passed`, error count `0`. Open both screenshots with the Read tool:
- `gui_a`: the whole window fits on screen, the Control Panel fills the left edge top to bottom, and Render
  Output (Earth, north up) fills the rest. No overlap.
- `gui_b`, after resizing to 1200×800: same arrangement, sidebar 400 px, image re-fitted.

If neither `xdotool` nor `wmctrl` is installed, resize the window by hand or skip the resize screenshot and
ledger that. Delete the screenshots afterwards.

- [ ] **Step 11: Commit (three commits, so the vendored code doesn't bury the real changes)**

```bash
git add .gitignore ext/imgui ext/glad
git commit -m "Vendor ImGui 1.91.9 + ImPlot 0.17 and glad (previously only in ext/ext.7z)"
git add CMakeLists.txt
git commit -m "Build Glimpse_gui on Linux: GLFW 3.4 via FetchContent, OpenGL::GL"
git add src/gui/layout.h src/gui/gl_res.cpp src/gui/app.cpp src/gui/ui.cpp tests/unit_tests/gui_layout_test.cpp \
        tests/testing.cpp CMakeLists.txt
git commit -m "Viewer: responsive window (80% of work area, sidebar + output layout follows resizes)"
```

---

### Task 7: Presets, docs, and the clean-clone check

**Files:**
- Modify: `CMakePresets.json`
- Modify: `CMakeLists.txt` (after `set(CMAKE_CXX_STANDARD 20)`)
- Modify: `.gitignore`
- Modify: `CLAUDE.md` (sections "Build & run" and "Known state on Linux")
- Modify: `docs/PLAN.md` (tick the Stage 0 boxes)

**Interfaces:** none new.

- [ ] **Step 1: `linux` preset; compile commands; ignore `refs/`**

`CMakePresets.json`: replace the `wsl` preset and keep `win`:

```json
{
    "version": 2,
    "configurePresets": [
        {
            "name": "linux",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/buildLinux",
            "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" }
        },
        {
            "name": "win",
            "generator": "Visual Studio 17 2022",
            "binaryDir": "${sourceDir}/build"
        }
    ]
}
```

In `CMakeLists.txt`, directly after `set(CMAKE_CXX_STANDARD 20)`, add:

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd / VS Code IntelliSense
```

In `.gitignore`, add a line `refs/`. Reference renders from technique 01 are cached there.

Update `configureAndBuild.sh` so its configure line reads `cmake --preset linux` (uncommented), followed by
`cmake --build buildLinux`.

- [ ] **Step 2: Clean-clone check (the Stage 0 "done when")**

A clone only contains committed work, so commit step 1 first:

```bash
git add CMakePresets.json CMakeLists.txt .gitignore configureAndBuild.sh
git commit -m "Linux preset, compile_commands.json, ignore refs/"
```

Then clone and check. `$SCRATCHPAD` is the session scratchpad directory.

```bash
rm -rf "$SCRATCHPAD/clone" && git clone -q . "$SCRATCHPAD/clone" && cd "$SCRATCHPAD/clone"
cmake --preset linux > /dev/null && cmake --build buildLinux 2>&1 | grep -E "error|FAILED"
ctest --test-dir buildLinux | tail -1
./buildLinux/Glimpse_cli --scene cornell_box --spp 64 --out x.png; echo "exit=$?"; ls -la x.png
ls buildLinux/Glimpse_gui buildLinux/compile_commands.json
cd - && rm -rf "$SCRATCHPAD/clone"
```
Expected: no build errors, `100% tests passed`, `exit=0` with a non-empty `x.png`, and both files listed.

- [ ] **Step 3: Update `CLAUDE.md`**

Replace the Linux block under "Build & run" with:

````markdown
Linux:
```bash
cmake --preset linux                      # Ninja, Release, buildLinux/ (first run downloads GLFW)
cmake --build buildLinux
./buildLinux/Glimpse_cli --list-scenes
./buildLinux/Glimpse_cli --scene cornell_box --spp 64 --width 300 --out results/cornell.png
./buildLinux/Glimpse_gui                  # viewer; run from the repo root (scenes load ./res)
ctest --test-dir buildLinux --output-on-failure
./buildLinux/Glimpse_tests "camera*"      # argv[1] is a boost.ut name filter
```
````

Replace the "Known state on Linux (checked 2026-10-03)" list with:

```markdown
State on Linux (Stage 0 done, 2026-10-10): the library, CLI, viewer and tests all build with GCC 11, and the
tests pass under ctest.
- CLI flags: `--scene <name|index> --spp --width --depth --seed --out --list-scenes --help`.
  - `--spp` is rounded down to a square (stratified grid).
  - `--seed` is reproducible on one machine, but not across thread counts (that is technique 03).
  - `--out` takes `.png/.jpg/.bmp/.tga` (PFM comes with technique 01).
- `Renderer::render_scene` calls `cam.initialize()` itself.
- `Image` rows are top-down, like image files. The renderer converts from camera `v` when writing, and the
  viewer does not flip.
- Scenes declare their own `scene.lights`.
- ImGui/ImPlot and glad are vendored in `ext/`, and GLFW comes from FetchContent.
- Line endings are LF (`.gitattributes`). `.git-blame-ignore-revs` lists the line-ending-only commits.
```

In the "Environment" section, change the `ext/` bullet to: "`ext/`: boost.ut, stb, ImGui+ImPlot and glad are
vendored. GLFW is fetched by CMake. `ext/ext.7z` and the prebuilt Windows GLFW are legacy."

- [ ] **Step 4: Tick Stage 0 in `docs/PLAN.md`**

Change each `- [ ]` under "Stage 0 — Foundation on Linux" to `- [x]`. If Task 8 is skipped, leave the
optional CI item unticked.

- [ ] **Step 5: Commit**

```bash
git add CLAUDE.md docs/PLAN.md
git commit -m "Docs: Stage 0 done — Linux build, CLI, viewer, tests"
```

- [ ] **Step 6: Vault notes (obsidian-kb skill)**

In `/mnt/d/ObsidianVault/1_TOPICS/Tech/Tech Projects/Glimpse/Glimpse - Stage 0 - Foundation.md`:
- Tick the four tasks and set `status: done`.
- Under "Learnings", add a dated `### Stage 0 (YYYY-MM-DD)` section with the real findings:
  - the `ut::cfg` ODR trap (an explicit variable-template specialisation must be `inline` and visible
    everywhere)
  - the CLI's black image (no `cam.initialize()`)
  - the duplicate Cornell light
  - the upside-down images (hidden by a display-time flip in the viewer)
  - the `--spp` square rounding
  - the GUI needing CMake changes only

In `1_TOPICS/Tech/Computer Graphics/Ray Tracing/Glimpse.md`, add an entry under "Progress log" that links
the stage note. Append only; never overwrite.

---

### Task 8 (optional): Linux CI job

The owner disabled push-triggered workflows on purpose (commit c6bf328), so this job is **manual-trigger
only**. `cpp-tests.yml` is broken (it targets branch `main` and runs `test.bat` on Ubuntu) and is replaced.

**Files:**
- Create: `.github/workflows/linux.yml`
- Delete: `.github/workflows/cpp-tests.yml`

- [ ] **Step 1: Write `.github/workflows/linux.yml`**

```yaml
name: Linux build + tests

on:
  workflow_dispatch:   # manual only; push triggers were disabled on purpose

jobs:
  build-and-test:
    runs-on: ubuntu-22.04   # GCC 11, same as the dev machine
    steps:
      - uses: actions/checkout@v4
      - name: Dependencies (GLFW X11 headers, GL, Ninja)
        run: sudo apt-get update && sudo apt-get install -y ninja-build xorg-dev libgl-dev
      - name: Configure
        run: cmake --preset linux
      - name: Build
        run: cmake --build buildLinux
      - name: Test
        run: ctest --test-dir buildLinux --output-on-failure
```

- [ ] **Step 2: Remove the broken workflow, validate the YAML, commit**

```bash
git rm .github/workflows/cpp-tests.yml
python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/linux.yml')); print('yaml ok')"
git add .github/workflows/linux.yml
git commit -m "CI: manual Linux build+test workflow; drop broken cpp-tests.yml"
```
Expected: `yaml ok`. Running it needs a push plus a manual trigger in GitHub, which is the owner's call. Tick
the optional CI box in `docs/PLAN.md` only after a green run.
