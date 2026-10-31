#!/bin/sh
# Record the three course scenes; raw CSV files include frame timing and particles.
set -eu
cd "$(dirname "$0")/.."
executable=${1:-./build/baikonur}
out=${2:-docs/evidence}
mkdir -p "$out"
scene() {
    stem=$1
    shift
    "$executable" --duration 12 --width 1280 --height 720 --no-retina --no-vsync \
        --csv "$out/$stem.csv" "$@" > "$out/$stem.log" 2>&1
    echo "Saved $out/$stem.csv"
}
scene ready
scene ignition_and_ascent --launch --start-time -3
scene separation_and_stars --launch --start-time 24
