#!/usr/bin/env python3
"""Regenerate the figures embedded in docs/ (and later docs/techniques/*.md).

Each figure is *computed* from the same formulas the C++ uses (the sampling warps in src/glimpse/util/sampling.h,
the stratified pixel offsets in src/glimpse/render.cpp, ...), so a change in a technique should be a change here
too. Output is deterministic SVG (fixed hash salt, fixed RNG seeds, no date, text kept as text), so regenerating
without changes gives a clean `git diff`. Same conventions as Glint's tools/diagrams.py.

  python3 -m venv tools/.venv && tools/.venv/bin/pip install -r tools/requirements.txt   # once
  tools/.venv/bin/python tools/diagrams.py                     # all figures
  tools/.venv/bin/python tools/diagrams.py cosine_hemisphere   # one figure
  tools/.venv/bin/python tools/diagrams.py --list
  tools/.venv/bin/python tools/diagrams.py --preview /tmp/figs # + PNG copies to look at
"""

import argparse
import math
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
from matplotlib.patches import FancyArrowPatch, Rectangle  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
FIGURES = {}  # name -> (output path relative to ROOT, function)

# Deterministic, theme-proof SVGs: white background (Obsidian/GitHub dark mode would otherwise show black text on
# black), text as <text> (smaller, searchable), fixed ids.
plt.rcParams.update({
    "svg.hashsalt": "glimpse",
    "svg.fonttype": "none",
    "font.size": 10,
    "axes.titlesize": 11,
    "figure.facecolor": "white",
    "axes.facecolor": "white",
    "savefig.facecolor": "white",
})

# Colors used across figures (one meaning each).
CAMERA_COLOR, IMAGE_COLOR = "#2b6cb0", "#c05621"   # camera/film space vs image/file space
UNIFORM_COLOR, COSINE_COLOR = "#718096", "#2f855a"  # uniform vs importance-sampled
LIGHT_COLOR = "#d69e2e"


def figure(name, path):
    def register(fn):
        FIGURES[name] = (path, fn)
        return fn
    return register


def save(fig, path, preview=None):
    out = ROOT / path
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, format="svg", bbox_inches="tight", dpi=150, metadata={"Date": None})  # dpi: rasterized layers
    if preview:  # PNG copies for a quick look (not committed)
        Path(preview).mkdir(parents=True, exist_ok=True)
        fig.savefig(Path(preview) / (out.stem + ".png"), dpi=110, bbox_inches="tight")
    plt.close(fig)
    print(f"  {path}")


def arrow(ax, start, end, color="black", **kw):
    ax.add_patch(FancyArrowPatch(start, end, arrowstyle="-|>", mutation_scale=12, color=color, lw=1.2, **kw))


# --- architecture / conventions --------------------------------------------------------------------------------

@figure("image_orientation", "docs/diagrams/image_orientation.svg")
def image_orientation():
    """Camera v and Film rows j count up from the bottom; Image rows (and PNG files) count down from the top.
    render.cpp writes Film row j into Image row H-1-j (the Stage 0 upside-down bug was a missing flip)."""
    H, W = 6, 4
    fig, axes = plt.subplots(1, 2, figsize=(8.2, 3.6))
    for ax, title, color, label in [(axes[0], "camera / Film:  j counts up  (v = j / (H−1))", CAMERA_COLOR, "j"),
                                    (axes[1], "Image / PNG file:  row 0 is the top", IMAGE_COLOR, "row")]:
        ax.set_title(title, color=color)
        for r in range(H):
            for c in range(W):
                ax.add_patch(Rectangle((c, r), 1, 1, fill=False, ec="#a0aec0", lw=0.8))
        # The same scene in both: a light near the top of the picture.
        ax.add_patch(Rectangle((1.2, H - 0.8), 1.6, 0.35, color=LIGHT_COLOR))
        ax.text(2.0, H - 1.25, "ceiling light", ha="center", va="center", fontsize=8, color="#744210")
        for r in range(H):
            idx = r if label == "j" else H - 1 - r  # y position r (0 = bottom) holds this index
            ax.text(-0.25, r + 0.5, f"{label} = {idx}", ha="right", va="center", fontsize=8, color=color)
        ax.set_xlim(-1.6, W + 0.2)
        ax.set_ylim(-0.4, H + 0.3)
        ax.set_aspect("equal")
        ax.axis("off")
    arrow(axes[0], (W + 0.05, 0.3), (W + 0.05, H - 0.3), color=CAMERA_COLOR)
    axes[0].text(W + 0.2, H / 2, "+v", color=CAMERA_COLOR, va="center")
    fig.text(0.5, -0.02, "render.cpp:  image.setFloat(i, imageHeight − 1 − j, ...)", ha="center", fontsize=9,
             family="monospace")
    return fig


# --- sampling ----------------------------------------------------------------------------------------------------

@figure("stratified_samples", "docs/diagrams/stratified_samples.svg")
def stratified_samples():
    """16 samples in one pixel: independent (randomDouble() - 0.5) vs stratified with jitter, as in render.cpp's
    sampleSquareStratified: offset = ((s + U) / sqrtSpp) - 0.5 for each sub-cell s."""
    spp, sqrt_spp = 16, 4
    rng = np.random.default_rng(7)
    independent = rng.random((spp, 2)) - 0.5
    cells = np.array([(si, sj) for sj in range(sqrt_spp) for si in range(sqrt_spp)], dtype=float)
    stratified = (cells + rng.random((spp, 2))) / sqrt_spp - 0.5

    fig, axes = plt.subplots(1, 2, figsize=(7.4, 3.8))
    for ax, pts, title in [(axes[0], independent, "independent: clumps and gaps"),
                           (axes[1], stratified, "stratified (4×4 cells, jittered)")]:
        if ax is axes[1]:
            for k in range(1, sqrt_spp):
                g = k / sqrt_spp - 0.5
                ax.axhline(g, color="#cbd5e0", lw=0.8)
                ax.axvline(g, color="#cbd5e0", lw=0.8)
        ax.add_patch(Rectangle((-0.5, -0.5), 1, 1, fill=False, ec="black", lw=1.2))
        ax.scatter(pts[:, 0], pts[:, 1], s=22, color=COSINE_COLOR if ax is axes[1] else UNIFORM_COLOR, zorder=3)
        ax.set_title(title)
        ax.set_xlim(-0.55, 0.55)
        ax.set_ylim(-0.55, 0.55)
        ax.set_aspect("equal")
        ax.set_xticks([-0.5, 0, 0.5])
        ax.set_yticks([-0.5, 0, 0.5])
    fig.suptitle("16 samples in one pixel (offsets from the pixel center)", y=1.0)
    return fig


@figure("cosine_hemisphere", "docs/diagrams/cosine_hemisphere.svg")
def cosine_hemisphere():
    """Uniform vs cosine-weighted hemisphere directions.

    sampling.h randomCosineDirection():  phi = 2π r1,  (x, y) = sqrt(r2) (cos phi, sin phi),  z = sqrt(1 - r2)
    Uniform over the hemisphere has density 1/(2π) per steradian, i.e. p(θ) = sin θ.
    Cosine-weighted has density cos θ / π per steradian (CosinePDF::value), i.e. p(θ) = 2 sin θ cos θ:
    more samples near the normal, where the Lambertian integrand cos θ is large."""
    n = 4000
    rng = np.random.default_rng(1)
    r1, r2 = rng.random(n), rng.random(n)
    phi = 2 * np.pi * r1
    cos_dirs = np.stack([np.cos(phi) * np.sqrt(r2), np.sin(phi) * np.sqrt(r2), np.sqrt(1 - r2)], axis=1)
    # Uniform on the hemisphere: z uniform in [0, 1] (Archimedes), phi uniform.
    zu, phiu = rng.random(n), 2 * np.pi * rng.random(n)
    ru = np.sqrt(1 - zu * zu)
    uni_dirs = np.stack([ru * np.cos(phiu), ru * np.sin(phiu), zu], axis=1)

    fig, axes = plt.subplots(1, 3, figsize=(11.5, 3.7))
    for ax, d, title, color in [(axes[0], uni_dirs, "uniform hemisphere\n(seen from above the normal)", UNIFORM_COLOR),
                                (axes[1], cos_dirs, "cosine-weighted\n(randomCosineDirection)", COSINE_COLOR)]:
        ax.scatter(d[:, 0], d[:, 1], s=1.5, color=color, alpha=0.6, rasterized=True)  # 4000 dots: keep the SVG small
        ax.add_patch(plt.Circle((0, 0), 1, fill=False, ec="black", lw=1))
        ax.set_title(title)
        ax.set_aspect("equal")
        ax.set_xlim(-1.05, 1.05)
        ax.set_ylim(-1.05, 1.05)
        ax.axis("off")

    ax = axes[2]
    theta = np.linspace(0, np.pi / 2, 200)
    bins = np.linspace(0, np.pi / 2, 25)
    ax.hist(np.arccos(uni_dirs[:, 2]), bins=bins, density=True, color=UNIFORM_COLOR, alpha=0.35, label="uniform samples")
    ax.hist(np.arccos(cos_dirs[:, 2]), bins=bins, density=True, color=COSINE_COLOR, alpha=0.35, label="cosine samples")
    ax.plot(theta, np.sin(theta), color=UNIFORM_COLOR, lw=2, label="sin θ")
    ax.plot(theta, 2 * np.sin(theta) * np.cos(theta), color=COSINE_COLOR, lw=2, label="2 sin θ cos θ")
    ax.set_xlabel("θ (angle from the normal)")
    ax.set_ylabel("density p(θ)")
    ax.set_xticks([0, math.pi / 8, math.pi / 4, 3 * math.pi / 8, math.pi / 2])
    ax.set_xticklabels(["0", "π/8", "π/4", "3π/8", "π/2"])
    ax.set_title("θ histograms vs analytic densities")
    ax.legend(fontsize=8, frameon=False)
    return fig


# --- color --------------------------------------------------------------------------------------------------------

def linear_to_srgb(x):
    """Same as src/glimpse/util/color.cpp linearToSRGB (IEC 61966-2-1)."""
    x = np.asarray(x, dtype=float)
    return np.where(x <= 0.0031308, 12.92 * x, 1.055 * np.power(np.maximum(x, 0), 1 / 2.4) - 0.055)


@figure("srgb_vs_sqrt", "docs/diagrams/srgb_vs_sqrt.svg")
def srgb_vs_sqrt():
    """Display encodings of linear radiance: sRGB (technique 01) vs RTIOW's sqrt ("gamma 2"). sRGB is brighter in the
    midtones (18% grey: 0.461 vs 0.424) and darker near black, where its linear toe (12.92 x) replaces the power curve's
    infinite slope."""
    x = np.linspace(0, 1, 500)
    fig, axes = plt.subplots(1, 2, figsize=(9.6, 4.0))
    ax = axes[0]
    ax.plot(x, linear_to_srgb(x), color=COSINE_COLOR, lw=2, label="sRGB (linearToSRGB)")
    ax.plot(x, np.sqrt(x), color=UNIFORM_COLOR, lw=2, ls="--", label="sqrt  (RTIOW, before 01)")
    ax.plot(x, x, color="#cbd5e0", lw=1, label="no encoding")
    for v in (0.18,):
        s, q = float(linear_to_srgb(v)), math.sqrt(v)
        ax.plot([v, v], [0, s], color="#a0aec0", lw=0.8, ls=":")
        ax.scatter([v, v], [s, q], color=[COSINE_COLOR, UNIFORM_COLOR], zorder=3, s=18)
        ax.annotate(f"18% grey: {s:.3f} vs {q:.3f}", (v, s), xytext=(0.3, 0.42), fontsize=8,
                    arrowprops=dict(arrowstyle="-", color="#a0aec0"))
    ax.set_xlabel("linear radiance (after exposure)")
    ax.set_ylabel("encoded value written to the 8-bit file")
    ax.set_title("full range")
    ax.legend(fontsize=8, frameon=False, loc="lower right")
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1.02)

    ax = axes[1]
    xs = np.linspace(0, 0.02, 400)
    ax.plot(xs, linear_to_srgb(xs), color=COSINE_COLOR, lw=2, label="sRGB")
    ax.plot(xs, np.sqrt(xs), color=UNIFORM_COLOR, lw=2, ls="--", label="sqrt")
    ax.axvline(0.0031308, color="#a0aec0", lw=0.8, ls=":")
    ax.text(0.0034, 0.005, "knee 0.0031308:\nlinear toe 12.92·x below", fontsize=8, color="#4a5568")
    ax.set_xlabel("linear radiance")
    ax.set_title("near black: sqrt lifts shadows (and their noise)")
    ax.legend(fontsize=8, frameon=False, loc="lower right")
    ax.set_xlim(0, 0.02)
    ax.set_ylim(0, 0.16)
    ax.set_xticks([0, 0.005, 0.01, 0.015, 0.02])
    return fig


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("names", nargs="*", help="figures to build (default: all)")
    parser.add_argument("--list", action="store_true")
    parser.add_argument("--preview", metavar="DIR", help="also write PNG copies into DIR")
    args = parser.parse_args()
    if args.list:
        for name, (path, _) in FIGURES.items():
            print(f"{name:20} {path}")
        return 0
    names = args.names or list(FIGURES)
    unknown = [n for n in names if n not in FIGURES]
    if unknown:
        print(f"unknown figure(s): {unknown}; --list shows them", file=sys.stderr)
        return 1
    for name in names:
        path, fn = FIGURES[name]
        save(fn(), path, args.preview)
    return 0


if __name__ == "__main__":
    sys.exit(main())
