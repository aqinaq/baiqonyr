#!/bin/sh
# Double-click on macOS, or run ./run.command in a desktop terminal.
set -eu
cd "$(dirname "$0")"
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake 3.20+ is required. See README.md for the course build setup."
    exit 1
fi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
exec ./build/baikonur
