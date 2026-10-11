#!/usr/bin/env bash
# Configure + build. Usage: ./build.sh [release|debug]   (default: release — a path tracer in Debug is painfully slow)
set -euo pipefail
cd "$(dirname "$0")"

cfg="${1:-release}"
case "$cfg" in
  release) type=Release ;;
  debug) type=Debug ;;
  *) echo "usage: $0 [release|debug]" >&2; exit 1 ;;
esac

cmake -S . -B "build/$cfg" -G Ninja -DCMAKE_BUILD_TYPE="$type"
cmake --build "build/$cfg"
