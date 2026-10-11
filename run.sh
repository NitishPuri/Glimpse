#!/usr/bin/env bash
# Run an app from the repo root (scenes load ./res, logs go to ./log_*.txt).
# Usage: ./run.sh <cli|gui|tests> [args...]
#   ./run.sh cli --scene cornell_box --spp 64
#   ./run.sh tests "camera*"          # boost.ut name filter
# CFG=debug ./run.sh ... runs the debug build (default: release).
set -euo pipefail
cd "$(dirname "$0")"

app="${1:-}"
case "$app" in
  cli|gui|tests) shift ;;
  *) echo "usage: $0 <cli|gui|tests> [args...]" >&2; exit 1 ;;
esac

exe="build/${CFG:-release}/Glimpse_$app"
[[ -x "$exe" ]] || { echo "$exe not built — run ./build.sh${CFG:+ $CFG} first" >&2; exit 1; }
exec "$exe" "$@"
