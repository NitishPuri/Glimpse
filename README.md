
```
 ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░▒▓██████████████▓▒░░▒▓███████▓▒░ ░▒▓███████▓▒░▒▓████████▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓█▓▒▒▓███▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓██████▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░▒▓█▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░▒▓█▓▒░
 ░▒▓██████▓▒░░▒▓████████▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓████████▓▒░
```

A physically-based path tracer that simulates light to reveal scenes with realism and depth.
"Reality is nothing but glimpses of light, shaped by perception and time."

[Glimpse001 - Demo](https://www.youtube.com/watch?v=AXzPrhivgw8)
![Glimpse1](https://github.com/user-attachments/assets/4127e19b-2338-4fc2-95c4-8d7031807a52)
![20250227142119_cornell_box_samples_1000](https://github.com/user-attachments/assets/5d8c7a27-4808-4a10-ad24-d26447f139f3)

~~Ray tracer in a weekend.~~ Glimpse. It grew out of my
[ray-tracer-in-a-weekend](https://github.com/NitishPuri/ray-tracer-in-a-weekend) and covers all three
*Ray Tracing in One Weekend* books. Now it is my **ray tracing learning path**:
- Every technique is learned and measured on the CPU first, reading
  [pbrt-v4](https://pbr-book.org/4ed/contents) and replicating *Ray Tracing Gems* articles.
- Later the same integrator is ported to the GPU, and the CPU renders become the reference images.
- Sibling project: [glint](../../glint), which follows GPU Gems in OpenGL and Vulkan.

- Plan and progress: [docs/PLAN.md](docs/PLAN.md)
- Context for AI assistants / contributors: [CLAUDE.md](CLAUDE.md)
- The 2025 roadmap (history): [ROADMAP.md](ROADMAP.md)

## Build & run (Linux)

Needs CMake ≥ 3.22, Ninja, a C++20 compiler (GCC 11 works) and the X11/GL dev headers for the viewer.

```bash
./build.sh                      # cmake -S . -B build/release -G Ninja; cmake --build build/release
./run.sh cli --list-scenes      # build/release/Glimpse_cli
./run.sh cli --scene cornell_box --spp 64 --width 300 --out results/cornell.png
./run.sh gui                    # build/release/Glimpse_gui — the interactive viewer
```

`./build.sh debug` / `CFG=debug ./run.sh ...` for a debug build. Release is the default because a path tracer
in Debug is painfully slow. `run.sh` always runs from the repo root, because scenes load `./res/...`.

CLI flags:

| Flag | Meaning |
|---|---|
| `--scene <name\|index>` | default `cornell_box`; `--list-scenes` shows them all |
| `--spp N` | samples per pixel, rounded down to a square (10 → 9) |
| `--width N` | height follows the scene's aspect ratio |
| `--depth N` | maximum bounces |
| `--seed N` | fixed RNG seed; reproducible on the same machine |
| `--out file` | `.png .jpg .bmp .tga`; default `results/<scene>.png` |

Tests: `./run.sh tests` (all), `./run.sh tests "camera*"` (boost.ut name filter), or
`ctest --test-dir build/release`.

Windows (the original 2025 setup, Visual Studio 2022): `configure.bat`, `build.bat`, `cli.bat`, `gui.bat`,
`test.bat <filter>`.

## Layout

- `src/core/`: the renderer library: camera, BVH, materials, pdfs, scenes, `Renderer`.
- `src/cli/`: `Glimpse_cli`, which renders a scene to an image file.
- `src/gui/`: `Glimpse_gui`, a GLFW + ImGui viewer with progressive rendering.
- `tests/`: boost.ut unit tests (`unit_tests/`) and end-to-end image tests (`e2e/`).
- `ext/`: vendored libraries (boost.ut, stb, Dear ImGui + ImPlot, glad). GLFW is fetched by CMake.

Dependencies: [stb](https://github.com/nothings/stb), [boost.ut](https://github.com/boost-ext/ut),
[Dear ImGui](https://github.com/ocornut/imgui) + [ImPlot](https://github.com/epezent/implot),
[GLFW](https://www.glfw.org), glad.

## Status

Stage 0 (foundation on Linux) is done. The tests, CLI, viewer and build scripts all work on Linux, and several
2025 bugs are fixed: upside-down images, the CLI's black renders, a crash on Save, and a hang on close. The
structure review comes next, then Stage 1, measuring (float output, reference renders, error metrics).
