#!/bin/bash
set -e

# --no-doxygen: skip the doc_doxygen ninja step (fast inner-loop build).
SKIP_DOXYGEN=0
for arg in "$@"; do
    case "$arg" in
        --no-doxygen) SKIP_DOXYGEN=1 ;;
        *) echo "build.sh: unknown arg: $arg" >&2; exit 1 ;;
    esac
done

# Firmware (ARM cross-compile)
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="$(pwd)/toolchain/arm-gcc-toolchain.cmake"
ninja -C build

# Documentation
if [[ $SKIP_DOXYGEN -eq 0 && -f doc/Doxyfile.in ]]; then
    ninja -C build doc_doxygen
fi
