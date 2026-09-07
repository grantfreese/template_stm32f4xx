#!/bin/bash
# install-packages.sh - install the apt and python packages needed to build the firmware
# copyright Grant Freese 2026
#
# One dependency is NOT an apt package and must be installed separately:
#   - arm-none-eabi GCC 10.3-2021.10: the firmware toolchain is a pinned release
#     unpacked into /opt (or ~/opt); do not substitute the apt gcc-arm-none-eabi.

set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    SUDO=sudo
else
    SUDO=
fi

PACKAGES=(
    # firmware build system
    cmake
    ninja-build
    build-essential
    git

    # python tooling and bench scripts
    python3
    python3-venv
    python3-pip
    python3-serial

    # formatting and static analysis (pinned to match .clang-format / .clang-tidy)
    clang-format-18
    clang-tidy-18

    # documentation (Doxyfile.in sets HAVE_DOT=YES, so graphviz is required)
    doxygen
    graphviz

    # CAN bench tooling
    can-utils

    # serial console to the target
    screen
    minicom
)

$SUDO apt-get update
$SUDO apt-get install -y --no-install-recommends "${PACKAGES[@]}"

echo
echo "apt and python packages installed."
echo
echo "Not apt-installable, install separately:"
echo "    - arm-none-eabi GCC 10.3-2021.10 (firmware toolchain, unpack into /opt)"
