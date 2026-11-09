#!/usr/bin/env bash
# Configure + build a preset (CMakePresets.json) into build/<preset>.
# Usage: ./build.sh [release|debug|clang-debug]   (default: release — a path tracer in Debug is painfully slow)
set -euo pipefail
cd "$(dirname "$0")"

cfg="${1:-release}"
case "$cfg" in
  release|debug) cxx=g++-14 ;;  # keep in sync with CMakePresets.json
  clang-debug) cxx=clang++-20 ;;
  *) echo "usage: $0 [release|debug|clang-debug]" >&2; exit 1 ;;
esac

# A build dir configured with another compiler (e.g. the old GCC 11 default) fails the first reconfigure after the
# preset switches compilers, so start that one from a fresh cache.
cache="build/$cfg/CMakeCache.txt"
if [[ -f "$cache" ]] && ! grep -q "^CMAKE_CXX_COMPILER:.*/$cxx\$" "$cache"; then
  echo "build/$cfg was configured with another compiler: removing its CMakeCache.txt"
  rm -rf "$cache" "build/$cfg/CMakeFiles"
fi

cmake --preset "$cfg"
cmake --build --preset "$cfg"
